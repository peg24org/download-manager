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
                       unique_ptr<FileIO> file_io)
  : request_manager(move(request_manager))
  , state_manager(state_manager)
  , timeout_seconds(5)
  , number_of_parts(1)
  , file_io(move(file_io))
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

void Downloader::set_download_parts(queue<pair<size_t, Chunk>> initial_parts)
{
  this->initial_parts = initial_parts;
}

void Downloader::set_parts(uint16_t parts)
{
  number_of_parts = parts;
}

void Downloader::run()
{
  transceiver = make_unique<HttpTransceiver>();
  init_connections();

  Buffer recv_buffer;
  rate.last_recv_time_point = steady_clock::now();
  const size_t kFileSize = state_manager->get_file_size();

  while (state_manager->get_total_recvd_bytes() < kFileSize) {
    struct timeval timeout = {.tv_sec=timeout_seconds, .tv_usec=0};
    check_new_sock_ops();
    int max_fd = set_descriptors();

    int sel_retval = select(max_fd + 1, &readfds, nullptr, nullptr, &timeout);
    if (sel_retval == -1)
      cerr << "Select error occurred." << endl;
    else if (sel_retval > 0) {
      for (auto& [index, connection] : connections) {
        if (connection.socket_ops.get() == nullptr) {
          cerr << " [ " << index << " ] " << " connection null." << endl;
          continue;
        }
        size_t recvd_bytes = 0;
        receive_from_connection(index, recv_buffer);
        recvd_bytes = recv_buffer.length();
        if (recvd_bytes) {
          // Write data
          size_t pos = connection.chunk.current;
          file_io->write(recv_buffer, pos);

          update_connection_stat(recvd_bytes, index);
          rate.total_recv_bytes += recvd_bytes;

          rate_process(rate, recvd_bytes);
          callback(rate.speed);
        }
      }   // End of for loop
      survey_connections();
    }   // End of else if
    else {    // Timeout
      // TODO: handle this
    }   // End of else for timeout
    // Check each connection for timeout
//    vector<int> timeout_indices = check_timeout();
//    if (timeout_indices.size() > 0)
//      retry(timeout_indices);
//    callback(rate.speed);
  }   // End of while loop
  request_manager->stop();
  request_manager->join();
}   // End of downloader thread run()

bool Downloader::regex_search_string(const string& input,
                                     const string& pattern,
                                     string& output, int pos_of_pattern)
{
  smatch m;
  regex e(pattern);
  bool retval = regex_search(input, m, e);
  output = m[pos_of_pattern];

  return retval;
}

bool Downloader::regex_search_string(const string& input, const string& pattern)
{
  string temp;

  return regex_search_string(input, pattern, temp);
}

bool Downloader::receive_data(Connection& connection, Buffer& buffer)
{
  bool result = true;
  int sock_desc = connection.socket_ops->get_socket_descriptor();
  int64_t recv_len = recv(sock_desc, buffer, buffer.capacity(), 0);

  if (recv_len >= 0)
    buffer.set_length(recv_len);
  else {
    connection.status = OperationStatus::SOCKET_RECV_ERROR;
    result = false;
    buffer.clear();
  }

  return result;
}

bool Downloader::send_data(const Connection& connection, const Buffer& buffer)
{
  size_t sent_bytes = 0;
  size_t tmp_sent_bytes = 0;
  int sock_desc = connection.socket_ops->get_socket_descriptor();

  while (sent_bytes < buffer.length()) {
    tmp_sent_bytes = send(sock_desc, const_cast<Buffer&>(buffer)+sent_bytes,
                          buffer.length(), 0);
    if (tmp_sent_bytes >= 0)
      sent_bytes += tmp_sent_bytes;
    else
      return false;
  }

  return true;
}

int Downloader::set_descriptors()
{
  int max_fd = 0;
  FD_ZERO(&readfds);
  for (auto& [index, connection] : connections) {
    if (connection.socket_ops.get() == nullptr) {
      continue;
    }
    int socket_desc = connection.socket_ops->get_socket_descriptor();
    FD_SET(socket_desc, &readfds);
    max_fd = (max_fd < socket_desc) ? socket_desc : max_fd;
  }

  return max_fd;
}

void Downloader::receive_from_connection(size_t _index, Buffer& buffer)
{
  buffer.clear();
  Connection& connection = connections[_index];

  int sock_desc = connection.socket_ops->get_socket_descriptor();

  if (FD_ISSET(sock_desc, &readfds)) {  // read from the socket
    if (!connection.header_skipped) {
      transceiver->receive(buffer, connection.socket_ops.get(), true);
      connection.header_skipped = true;
    }
    else {
      transceiver->receive(buffer, connection.socket_ops.get(), false);
    }
  }
}

void Downloader::init_connections()
{
  for (uint16_t i = 0; i < number_of_parts; ++i) {
    if (state_manager->part_available())
      init_connection(i);
    else
      {/*part not available.*/}
  }
  request_manager->start();
}

void Downloader::init_connection(size_t connection_index)
{
  pair<size_t, Chunk> part = state_manager->get_part();
  const size_t start = part.second.current;

  const size_t length = part.second.end - part.second.current;
  const uint16_t index = state_manager->downloading_parts();
  connections[connection_index].chunk.end = part.second.end;
  connections[connection_index].chunk.current = start;
  request_manager->add_request(start, part.second.end, connection_index);
}

Connection::Status Downloader::create_connection(bool info_connection)
{
}

vector<int> Downloader::check_timeout()
{
  vector<int> result;
  for (auto& connection_it : connections) {
    Connection& connection = connection_it.second;
    steady_clock::time_point now = steady_clock::now();
    duration<double> time_span =
      duration_cast<duration<double>>(now - connection.last_recv_time_point);
    if (time_span.count() > timeout_seconds)
      // Timeout index
      result.push_back(connection_it.first);
  }

  return result;
}

void Downloader::retry(const vector<int>& connection_indices)
{
  for (const int index : connection_indices) {
    Connection& connection = connections[index];
    //init_connection(connection);
    send_request(connection);
  }
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

void Downloader::update_connection_stat(size_t recvd_bytes, size_t index)
{
  connections[index].chunk.current += recvd_bytes;
  connections[index].last_recv_time_point = steady_clock::now();
  state_manager->update(index, recvd_bytes);
}

void Downloader::survey_connections()
{
  // Remove finished connections
  vector<size_t> finished_connections;
  for (auto& [index, connection] : connections)
    if (connection.chunk.current >= connection.chunk.end)
      finished_connections.push_back(index);

  for (size_t index : finished_connections)
    connections.erase(index);
  // Create new connections
  while (connections.size() < number_of_parts && state_manager->part_available())
    init_connection(connections.size());
}

void Downloader::on_dwl_available(uint16_t index,
                                  unique_ptr<SocketOps> sock_ops)
{
  lock_guard<mutex> lock(new_available_parts_mutex);
  new_available_parts.push({index, move(sock_ops)});
}

void Downloader::check_new_sock_ops()
{
  lock_guard<mutex> lock(new_available_parts_mutex);
  while (!new_available_parts.empty()) {
    NewAvailPart& new_available_part = new_available_parts.front();
    connections[new_available_part.part_index].socket_ops = move(
        new_available_part.sock_ops);
    new_available_parts.pop();
  }
}

