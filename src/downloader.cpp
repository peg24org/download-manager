#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <mutex>
#include <regex>
#include <cstdlib>
#include <cassert>

#include "node.h"
#include "downloader.h"

using namespace std;
using namespace std::chrono;

const string Downloader::HTTP_HEADER =
    "(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)";

DownloadStateManager* Downloader::download_state_manager = nullptr;

Downloader::Downloader(const struct DownloadSource& download_source)
  : download_source(download_source),
    number_of_connections(1)
{
}

Downloader::Downloader(const struct DownloadSource& download_source,
                       std::unique_ptr<Writer> writer,
                       const ChunksCollection& chunks_collection,
                       time_t timeout_seconds,
                       int number_of_connections)
  : download_source(download_source)
  , writer(move(writer))
  , chunks_collection(chunks_collection)
  , timeout_seconds(timeout_seconds)
  , number_of_connections(number_of_connections)
{
}

void Downloader::run()
{
  init_connections();

  bool request_sent = true;
  if (!send_requests()) {
    request_sent = false;
    cerr << "Sending request failed." << endl;
  }

  static constexpr size_t kBufferLen = 40000;
  char recv_buffer[kBufferLen];
  const size_t kFileSize = writer->get_file_size();

  while (request_sent && (writer->get_total_written_bytes() < kFileSize)) {
    struct timeval timeout = {.tv_sec=timeout_seconds, .tv_usec=0};
    int max_fd = set_descriptors();

    int sel_retval = select(max_fd+1, &readfds, nullptr, nullptr, &timeout);
    if (sel_retval == -1)
      cerr << "Select error occurred." << endl;
    else if (sel_retval > 0) {
      for (size_t index = 0; index < connections.size(); ++index) {
        size_t recvd_bytes = 0;
        recvd_bytes = receive_from_connection(index, recv_buffer, kBufferLen);
        if (recvd_bytes) {
          size_t pos = connections[index].chunk.current_pos;
          writer->write(recv_buffer, recvd_bytes, pos, index);
          connections[index].chunk.current_pos += recvd_bytes;
          connections[index].last_recv_time_point = steady_clock::now();
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

bool Downloader::receive_data(Connection& connection, char* buffer,
                              size_t& received_len, size_t buffer_capacity)
{
  bool result = true;
  int64_t recv_len = recv(connection.socket_ops->get_socket_descriptor(),
                          buffer, buffer_capacity, 0);

  if (recv_len >= 0)
    received_len = recv_len;
  else {
    connection.status = OperationStatus::SOCKET_RECV_ERROR;
    result = false;
    received_len = 0;
  }

  return result;
}

bool Downloader::send_data(Connection& connection, const char* buffer,
                           size_t len)
{
  size_t sent_bytes = 0;
  size_t tmp_sent_bytes = 0;

  int sock_desc = connection.socket_ops->get_socket_descriptor();
  while (sent_bytes < len) {
    if ((tmp_sent_bytes = send(sock_desc, buffer, len, 0)) > 0)
      sent_bytes += tmp_sent_bytes;
    else
      return false;
  }
  return true;
}

bool Downloader::init_connections()
{
  bool result = true;
  for (int index = 0; index < number_of_connections; ++index)
    result &= init_connection(connections[index]);
  return result;
}

bool Downloader::init_connection(Connection& connection)
{
  int result;
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

  connection.status = OperationStatus::NOT_STARTED;

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
    connection.status = OperationStatus::NOT_STARTED;
    init_connection(connection);
    send_request(connection);
  }
}
