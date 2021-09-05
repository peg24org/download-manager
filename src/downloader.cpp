#include <sys/socket.h>

#include <regex>

#include "node.h"
#include "buffer.h"
#include "downloader.h"
#include "request_manager.h"

using namespace std;
using namespace std::chrono;

const string Downloader::HTTP_HEADER =
    "(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)";

Downloader::Downloader(unique_ptr<RequestManager> request_manager,
                       shared_ptr<StateManager> state_manager,
                       unique_ptr<FileIO> file_io,
                       unique_ptr<Transceiver> transceiver)
  : timeout({.tv_sec=0, .tv_usec=100'000})
  , timeout_seconds(5)
  , number_of_parts(1)
  , file_io(move(file_io))
  , transceiver(move(transceiver))
  , wait_first_conn_response(true)
  , state_manager(state_manager)
  , request_manager(move(request_manager))
  , connection_mngr(state_manager)
{
  DwlAvailNotifyCB callback = bind(&Downloader::on_dwl_available, this,
                                   placeholders::_1, placeholders::_2);
  this->request_manager->register_dwl_notify_cb(callback);
}

void Downloader::register_callback(CallBack callback)
{
  this->callback = callback;
}

void Downloader::set_speed_limit(size_t speed_limit)
{
  rate.limit = speed_limit;
}

void Downloader::set_parts(uint16_t parts)
{
  number_of_parts = parts;
}

void Downloader::run()
{
  init_connections();
  // TODO: implement using conditional variable.
  while (wait_first_conn_response)
    this_thread::sleep_for(milliseconds(10));

  Buffer recv_buffer;
  rate.last_recv_time_point = steady_clock::now();
  const size_t kFileSize = state_manager->get_file_size();

  while (state_manager->get_total_recvd_bytes() < kFileSize) {
    //cout << "in loop" << endl;
    check_new_sock_ops();
    //survey_connections();
    connection_mngr.survey_connections();
    int max_fd = set_descriptors();
    int sel_retval = select(max_fd + 1, &readfds, nullptr, nullptr, &timeout);
    if (sel_retval == -1)
      cerr << "Select error occurred." << endl;
    else if (sel_retval == 0)
      continue;
    else if (sel_retval > 0) {
      timeout = {.tv_sec=timeout_seconds, .tv_usec=100'000};
      vector<uint16_t> indices_list = connection_mngr.get_indices_list();
      for (uint16_t index: indices_list) {
        if (connection_mngr.get_sock_ops(index) == nullptr) {
          continue;
        }
        size_t recvd_bytes = 0;
        receive_from_connection(index, recv_buffer);
        recvd_bytes = recv_buffer.length();
        if (recvd_bytes) {
          // Write data
          size_t pos = connection_mngr.get_current_pos(index);
          file_io->write(recv_buffer, pos);

          state_manager->update(index, recvd_bytes);
          rate.total_recv_bytes += recvd_bytes;

          rate_process(rate, recvd_bytes);
          callback(rate.speed);
        }
      }   // End of for loop
    }   // End of else if
    else {    // Timeout
      // TODO: handle this
    }   // End of else for timeout
    // Check each connection for timeout
//    vector<int> timeout_indices = check_timeout();
//    if (timeout_indices.size() > 0)
// TODO:     retry(timeout_indices);
//    callback(rate.speed);
    //survey_connections();
  }   // End of while loop

  request_manager->stop();
  request_manager->join();
}   // End of downloader thread run()

int Downloader::set_descriptors()
{
  int max_fd = 0;
  FD_ZERO(&readfds);
  vector<uint16_t> indices_list = connection_mngr.get_indices_list();
  for (uint16_t i : indices_list) {
    const SocketOps* socket_ops = connection_mngr.get_sock_ops(i);
    if (socket_ops == nullptr)
      continue;
    int socket_desc = socket_ops->get_socket_descriptor();
    FD_SET(socket_desc, &readfds);
    max_fd = (max_fd < socket_desc) ? socket_desc : max_fd;
  }

  return max_fd;
  //int max_fd = 0;
  //FD_ZERO(&readfds);
  //for (auto& [index, connection] : connections) {
  //  if (connection.socket_ops.get() == nullptr || connection.scheduled)
  //    continue;
  //  int socket_desc = connection.socket_ops->get_socket_descriptor();
  //  FD_SET(socket_desc, &readfds);
  //  max_fd = (max_fd < socket_desc) ? socket_desc : max_fd;
  //}

  //return max_fd;
}

void Downloader::receive_from_connection(size_t _index, Buffer& buffer)
{
  buffer.clear();

  SocketOps* sock_ops = connection_mngr.get_sock_ops(_index);
  int sock_desc = sock_ops->get_socket_descriptor();
  if (FD_ISSET(sock_desc, &readfds)) {
    transceiver->receive(buffer, sock_ops,
                         connection_mngr.get_header_skipped_stat(_index));
  }
}

void Downloader::init_connections()
{
//  state_manager->set_chunks_num(number_of_parts);
//  vector<uint16_t> parts_list = state_manager->get_parts();
//  for (uint16_t i : parts_list) {
//      const size_t start = state_manager->get_start_pos(i);
//      connections[i] = Connection();
//  }

  connection_mngr.set_parts_max(number_of_parts);
  connection_mngr.init();
  vector<uint16_t> indices_list = connection_mngr.get_indices_list();
  for (uint16_t i: indices_list)
    init_connection(i);
  request_manager->start();
//  for (uint16_t i = 0; i < number_of_parts; ++i) {
//    if (state_manager->part_available())
//      init_connection();
//    else
//      {/*part not available.*/}
//  }
//  request_manager->start();
}

void Downloader::init_connection(uint16_t index)
{
    size_t end = connection_mngr.get_end_pos(index);
    size_t current = connection_mngr.get_current_pos(index);
    size_t start = connection_mngr.get_start_pos(index);
    cout << "init :" << index << " " << start << " " << current << endl;
    request_manager->add_request(current, end, index);
    connection_mngr.set_init_stat(true, index);
}

//vector<int> Downloader::check_timeout()
//{
//  vector<int> result;
//  for (auto& connection_it : connections) {
//    Connection& connection = connection_it.second;
//    steady_clock::time_point now = steady_clock::now();
//    duration<double> time_span =
//      duration_cast<duration<double>>(now - connection.last_recv_time_point);
//    if (time_span.count() > timeout_seconds)
//      // Timeout index
//      result.push_back(connection_it.first);
//  }
//
//  return result;
//}

//void Downloader::retry(const vector<int>& connection_indices)
//{
//  for (const int index : connection_indices) {
//    Connection& connection = connections[index];
//    init_connection(connection);
//    send_request(connection);
//  }
//}

void Downloader::rate_process(RateParams& rate, size_t recvd_bytes)
{
  // Speed limit
  if (rate.limit > 0) {
    rate.total_limiter_bytes += recvd_bytes;
    double limit_duration = duration_cast<milliseconds>(steady_clock::now() - rate.last_recv_time_point).count();
    if (rate.total_limiter_bytes > rate.limit && limit_duration < 1000) {
      const size_t delay = 1000 * ((static_cast<double>(rate.total_limiter_bytes)) / rate.limit);
      this_thread::sleep_for(chrono::milliseconds(delay));
      rate.total_limiter_bytes = 0;
    }
  }

  // Speed computes
  double duration = duration_cast<milliseconds>(
      steady_clock::now() - rate.last_recv_time_point).count();
  if (duration > 1000) {
    double bytes_diff = rate.total_recv_bytes - rate.last_overall_recv_bytes;
    rate.speed = (1000 * bytes_diff) / duration;
    rate.last_recv_time_point = steady_clock::now();
    rate.last_overall_recv_bytes = rate.total_recv_bytes;
    rate.total_limiter_bytes = 0;
  }
}

void Downloader::update_connection_stat(size_t recvd_bytes, size_t index)
{
  //if (connections[index].chunk.current + recvd_bytes > connections[index].chunk.end)
  //  recvd_bytes = connections[index].chunk.end - connections[index].chunk.current + 1;

  //connections[index].chunk.current += recvd_bytes;
  //connections[index].last_recv_time_point = steady_clock::now();
  state_manager->update(index, recvd_bytes);
}

void Downloader::survey_connections()
{
//  // Remove finished connections
//  vector<size_t> finished_connections;
//  for (auto& [index, connection] : connections) {
//    if (connection.scheduled == true)
//      continue;
//    const int64_t rem_len = connection.chunk.end - connection.chunk.current;
//    if ( rem_len <= 0)
//      finished_connections.push_back(index);
//  }
//  for (size_t index : finished_connections)
//    connections.erase(index);
//
//
//  uint16_t active_connections = 0;
//  for (auto& [index, connection] : connections)
//    if (!connection.scheduled)
//      active_connections++;
//  if (active_connections < number_of_parts)
//    for (auto& [_, connection] : connections)
//      if (connection.scheduled)
//        connection.scheduled = false;
//
//  for (auto& [_, connection] : connections) {
//    if (connection.scheduled || connection.substitute_created)
//      continue;
//    const int64_t delta = connection.chunk.current - connection.chunk.start;
//    const int64_t len = connection.chunk.end - connection.chunk.start;
//
//    if (delta >= 6 * len / 10) {
//      if (state_manager->part_available())
//        init_connection(true);
//      connection.substitute_created = true;
//    }
//  }
//
//  if (connections.size() < number_of_parts)
//    if (state_manager->part_available())
//      init_connection();
}

void Downloader::on_dwl_available(uint16_t index,
                                  unique_ptr<SocketOps> sock_ops)
{
  cout << __FUNCTION__ << endl;
  lock_guard<mutex> lock(new_available_parts_mutex);
  new_available_parts.push({index, move(sock_ops)});
  if (wait_first_conn_response)
    wait_first_conn_response = false;
}

void Downloader::check_new_sock_ops()
{
  lock_guard<mutex> lock(new_available_parts_mutex);
  while (!new_available_parts.empty()) {
    NewAvailPart& new_available_part = new_available_parts.front();
    size_t index = new_available_part.part_index;
    connection_mngr.set_sock_ops(move(new_available_part.sock_ops), index);
//    connections[new_available_part.part_index].socket_ops = move(
//        new_available_part.sock_ops);
    new_available_parts.pop();
  }
}

