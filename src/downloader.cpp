#include <arpa/inet.h>
#include <sys/socket.h> 

#include <mutex>
#include <regex>
#include <cstdlib>
#include <cassert>

#include "node.h"
#include "downloader.h"

using namespace std;

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
                       long int timeout_seconds,
                       int number_of_connections)
  : download_source(download_source)
  , writer(move(writer))
  , chunks_collection(chunks_collection)
  , timeout({.tv_sec=timeout_seconds, .tv_usec=0})
  , buffer_offset(0)
  , number_of_connections(number_of_connections)
{
}

void Downloader::run()
{
  init_connections();
  send_request();

  static constexpr size_t kBufferLen = 40000;
  char recv_buffer[kBufferLen];
  const size_t kFileSize = writer->get_file_size();

  while (writer->get_total_written_bytes() < kFileSize) {

    int max_fd = set_descriptors();

    int sel_retval = select(max_fd+1, &readfds, NULL, NULL, &timeout);
    if (sel_retval == -1)
      cerr << "Select error occurred." << endl;
    else if (sel_retval > 0) {
      for (size_t index = 0; index < connections.size(); ++index) {
        size_t recvd_bytes = receive_from_connection(index, recv_buffer,
                                                     kBufferLen);
        if (recvd_bytes) {
          writer->write(recv_buffer+buffer_offset, recvd_bytes,
                        connections[index].chunk.current_pos, index);
          connections[index].chunk.current_pos += recvd_bytes;
        }
      }   // End of for loop
    }   // End of else if
    else {    // Timeout
      break;    // Break while loop
    }   // End of else for timeout
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
  received_len = recv(connection.socket_ops->get_socket_descriptor(),
                      buffer, buffer_capacity, 0);
  if (received_len < 0) {
    connection.status = OperationStatus::SOCKET_RECV_ERROR;
    return false;
  }

  return true;
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
  // TODO handle errors.
  for (int index = 0; index < number_of_connections; ++index) {
    unique_ptr<SocketOps> socket_ops = make_unique<SocketOps>(
        download_source.ip, download_source.port);
    socket_ops->connect();
    connections[index].socket_ops = move(socket_ops);
  }

  return true;
}
