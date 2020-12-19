#include <sys/socket.h>

#include <regex>

#include "node.h"
#include "buffer.h"
#include "downloader.h"

using namespace std;
using namespace std::chrono;

const string Downloader::HTTP_HEADER =
    "(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)";

Downloader::Downloader(const struct DownloadSource& download_source)
  : download_source(download_source),
    number_of_parts(1)
{
}

Downloader::Downloader(const struct DownloadSource& download_source,
                       std::unique_ptr<Writer> writer,
                       const ChunksCollection& chunks_collection,
                       time_t timeout_seconds,
                       int number_of_parts)
  : download_source(download_source)
  , writer(move(writer))
  , chunks_collection(chunks_collection)
  , timeout_seconds(timeout_seconds)
  , number_of_parts(number_of_parts)
{
  for (auto chunk : chunks_collection) {
    connections[chunk.first].chunk = chunk.second;
    connections[chunk.first].status = OperationStatus::NOT_STARTED;
  }
}

void Downloader::register_callback(CallBack callback)
{
  this->callback = callback;
}

void Downloader::set_speed_limit(size_t speed_limit)
{
  rate.limit = speed_limit;
}

void Downloader::run()
{
  init_connections();

  bool request_sent = true;
  if (!send_requests()) {
    request_sent = false;
    cerr << "Sending request failed." << endl;
  }

  Buffer recv_buffer;
  const size_t kFileSize = writer->get_file_size();

  rate.last_recv_time_point = steady_clock::now();

  while (request_sent && (writer->get_total_written_bytes() < kFileSize)) {
    struct timeval timeout = {.tv_sec=timeout_seconds, .tv_usec=0};
    int max_fd = set_descriptors();

    int sel_retval = select(max_fd+1, &readfds, nullptr, nullptr, &timeout);
    if (sel_retval == -1)
      cerr << "Select error occurred." << endl;
    else if (sel_retval > 0) {
      for (size_t index = 0; index < connections.size(); ++index) {
        size_t recvd_bytes = 0;
        receive_from_connection(index, recv_buffer);
        recvd_bytes = recv_buffer.length();
        if (recvd_bytes) {
          // Write data
          size_t pos = update_connection_stat(connections[index], recvd_bytes);

          writer->write(recv_buffer, pos, index);

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
    vector<int> timeout_indices = check_timeout();
    if (timeout_indices.size() > 0)
      retry(timeout_indices);

    callback(rate.speed);
  }   // End of while loop
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

bool Downloader::init_connections()
{
  bool result = true;
  for (int index = 0; index < number_of_parts; ++index)
    result &= init_connection(connections[index]);
  return result;
}

bool Downloader::init_connection(Connection& connection)
{
  bool result;
  if (!download_source.proxy_ip.empty()) {
    connection.socket_ops = make_unique<SocketOps>(download_source.proxy_ip,
                                                   download_source.proxy_port);
    result = connection.socket_ops->connect();
    // TODO: handle return result.
    int socket_descriptor = connection.socket_ops->get_socket_descriptor();
    connection.http_proxy = make_unique<HttpProxy>(download_source.host_name,
                                                   download_source.port);
    connection.http_proxy->connect(socket_descriptor);
  }
  else {
    connection.socket_ops = make_unique<SocketOps>(download_source.ip,
                                                   download_source.port);
    result = connection.socket_ops->connect();
  }

  connection.header_skipped = false;

  return result;
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
    init_connection(connection);
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


size_t Downloader::update_connection_stat(Connection& connection,
                                          size_t recvd_bytes)
{
  size_t pos = connection.chunk.current_pos;
  connection.chunk.current_pos += recvd_bytes;
  connection.last_recv_time_point = steady_clock::now();

  return pos;
}
