#include "http_downloader.h"

#include <regex>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>

#include "node.h"

using namespace std;

HttpDownloader::HttpDownloader(const struct DownloadSource& download_source)
  : Downloader(download_source)
{
}

HttpDownloader::HttpDownloader(const struct DownloadSource& download_source,
                               std::unique_ptr<Writer> writer,
                               ChunksCollection& chunks_collection,
                               long int timeout,
                               int number_of_connections)
  : Downloader(download_source, move(writer), chunks_collection, timeout,
               number_of_connections)
{
  for (auto chunk : chunks_collection) {
    connections[chunk.first].chunk = chunk.second;
    connections[chunk.first].status = OperationStatus::NOT_STARTED;
  }
}

size_t HttpDownloader::get_size()
{
  string size_string;
  if (regex_search_string(receive_header, "(Content-Length: )(\\d+)",
                          size_string))
    return  strtoul(static_cast<const char*>(size_string.c_str()), nullptr, 0);
  return 0;
}

bool HttpDownloader::check_redirection(string& redirecting)
{
  if (regex_search_string(receive_header, HTTP_HEADER, redirecting)) {
    // Check redirection
    if (regex_search_string(receive_header, "(Location: )(.+)",redirecting))
      return true;
    return false;
  }
  return false;
}

int HttpDownloader::check_link(string& redirected_url, size_t& file_size)
{
  int redirect_status = 0;

  init_connections();
  // Use GET instead of HEAD, because some servers doesn't support HEAD
  //  command.
  string request =
    "GET " + download_source.file_path + download_source.file_name +
    " HTTP/1.1\r\nHost: " + download_source.host_name + ":" +
    to_string(download_source.port) + "\r\n\r\n";

  receive_header.resize(MAX_HTTP_HEADER_LENGTH);
  if (!send_data(connections[0], request.c_str(), request.length()))
    return -1;

  size_t len = 0;
  unique_ptr<char[]> buffer = make_unique<char[]>(256 * 1024 * sizeof(char));
  while (true) {
    size_t number_of_bytes;
    if (!receive_data(connections[0], const_cast<char*>(receive_header.data()),
                      number_of_bytes, MAX_HTTP_HEADER_LENGTH))
      return -1;
    len += number_of_bytes;
    if (receive_header.find("\r\n\r\n") != string::npos) {
      break;
    }
  }

  if (check_redirection(redirected_url))
    // Link is redirected
    redirect_status = 1;
  else {
    file_size = get_size();
    // Link is not redirected
    redirect_status = 0;
  }

  return redirect_status;
}

size_t HttpDownloader::get_header_delimiter_position(const char* buffer)
{
  char* pos = const_cast<char*>(strstr(buffer,"\r\n\r\n"));
  if(pos) {
    string recv_buffer = string(buffer);
    smatch m;
    regex e("(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)");
    bool found = regex_search(recv_buffer, m, e);
    if(found) {
      // If response not equal to 200,OK
      if(stoi(m[2].str())/100 != 2) {
        pos = NULL;
      }
    }
    else {
      pos = NULL;
    }
  }
  return pos - buffer;
}

void HttpDownloader::send_request()
{
  for (size_t index = 0; index < connections.size(); ++index) {
    string request = "GET " + download_source.file_path + "/" +
      download_source.file_name + " HTTP/1.1\r\nRange: bytes=" +
      to_string(chunks_collection[index].current_pos) + "-" +
      to_string(chunks_collection[index].end_pos) + "\r\n" +  "Host:" +
      download_source.host_name +	":" + to_string(download_source.port) +
      "\r\n\r\n";
    if(!send_data(connections[index], request.c_str(), request.length()))
      connections[index].status = OperationStatus::SOCKET_SEND_ERROR;
  }
}

int HttpDownloader::set_descriptors()
{
  int max_fd = 0;
  FD_ZERO(&readfds);

  for (size_t index = 0; index < connections.size(); ++index) {
    int sock_desc = connections[index].socket_ops->get_socket_descriptor();
    FD_SET(sock_desc, &readfds);
    max_fd = (max_fd < sock_desc) ? sock_desc : max_fd;
  }

  return max_fd;
}

size_t HttpDownloader::receive_from_connection(size_t index, char* buffer,
                                               size_t buffer_capacity)
{
  size_t recvd_bytes = 0;
  int sock_desc = connections[index].socket_ops->get_socket_descriptor();

  if (FD_ISSET(sock_desc, &readfds)) {  // read from the socket
    receive_data(connections[index], buffer,  recvd_bytes, buffer_capacity);

    // Skip the HTTP header
    if (connections[index].status == OperationStatus::NOT_STARTED) {
      buffer_offset = get_header_delimiter_position(buffer) + 4;
      recvd_bytes -= buffer_offset;
      connections[index].status = OperationStatus::DOWNLOADING;
    }
    else
      buffer_offset = 0;
  }

  return recvd_bytes;
}
