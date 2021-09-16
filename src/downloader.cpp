#include <sys/socket.h>

#include <regex>
#include <iostream>

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
  , pause(false)
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

void Downloader::pause_download()
{
  pause = true;
}

void Downloader::resume_download()
{
  pause = false;
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
    if (pause) {
      this_thread::sleep_for(milliseconds(200));
      continue;
    }
    { // check connection if not inited.
      vector<uint16_t> indices_list = connection_mngr.get_indices_list();
      for (uint16_t index : indices_list)
        if (connection_mngr.get_init_stat(index) == false)
          init_connection(index);
    }
    check_new_sock_ops();
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
  //connection_mngr.set_parts_max(number_of_parts);
  connection_mngr.init();
  vector<uint16_t> indices_list = connection_mngr.get_indices_list();

  for (uint16_t i: indices_list)
    init_connection(i);
  request_manager->start();
}

void Downloader::init_connection(uint16_t index)
{
    size_t end = connection_mngr.get_end_pos(index);
    size_t current = connection_mngr.get_current_pos(index);
    request_manager->add_request(current, end, index);
    connection_mngr.set_init_stat(true, index);
}

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

void Downloader::on_dwl_available(uint16_t index,
                                  unique_ptr<SocketOps> sock_ops)
{
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
    new_available_parts.pop();
  }
}

