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

Downloader::Downloader(const struct DownloadSource& download_source,
                       const vector<int>& socket_descriptors)
  : download_source(download_source) , socket_descriptors(socket_descriptors)
{
}

Downloader::Downloader(const struct DownloadSource& download_source,
                       const std::vector<int>& socket_descriptors,
                       std::unique_ptr<Writer> writer,
                       const ChunksCollection& chunks_collection,
                       time_t timeout_seconds)
  : download_source(download_source)
  , writer(move(writer))
  , chunks_collection(chunks_collection)
  , socket_descriptors(socket_descriptors)
  , timeout_interval({.tv_sec=timeout_seconds, .tv_usec=0})
{
}

void Downloader::run()
{
  downloader_trd();
}

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
  received_len = recv(connection.sock_desc, buffer, buffer_capacity,0);
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

  while (sent_bytes < len) {
    if ((tmp_sent_bytes = send(connection.sock_desc, buffer, len, 0)) > 0)
      sent_bytes += tmp_sent_bytes;
    else
      return false;
  }
  return true;
}
