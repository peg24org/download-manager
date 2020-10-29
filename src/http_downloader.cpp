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
  bool result = false;
  // Check header
  if (regex_search_string(receive_header, HTTP_HEADER)) {
    // Check redirection
    if (regex_search_string(receive_header, "(Location: )(.+)",redirecting)) {
      result = true;
      // Check if redirecting to path instead of URL, then create new URL.
      if (!regex_search_string(receive_header, "http.*://")) {
        const Protocol& protocol = download_source.protocol;
        string prefix = protocol == Protocol::HTTP ? "http://" : "https://";
        redirecting = prefix + download_source.host_name + redirecting;
      }
    }
  }
  return result;
}

int HttpDownloader::check_link(string& redirected_url, size_t& file_size)
{
  int redirect_status = 0;

  if (!init_connections())
    return -1;
  // Use GET instead of HEAD, because some servers doesn't support HEAD
  // command.
  string request =
    "GET " + download_source.file_path + download_source.file_name +
    " HTTP/1.1\r\nHost: " + download_source.host_name + ":" +
    to_string(download_source.port) + "\r\n\r\n";

  receive_header.resize(MAX_HTTP_HEADER_LENGTH);
  if (!send_data(connections[0], request.c_str(), request.length()))
    return -1;

  while (true) {
    size_t number_of_bytes;
    if (!receive_data(connections[0], const_cast<char*>(receive_header.data()),
                      number_of_bytes, MAX_HTTP_HEADER_LENGTH))
      return -1;
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
  if (pos == NULL)
    return 0;
  return pos - buffer;
}

void HttpDownloader::send_request()
{
  for (size_t index = 0; index < connections.size(); ++index) {
    Connection& connection = connections[index];
    const string port = to_string(download_source.port);
    const string host_name = download_source.host_name;
    string current_pos;
    if (connection.chunk.current_pos > 0)
      current_pos = to_string(connections[index].chunk.current_pos);
    else
      current_pos = to_string(connections[index].chunk.current_pos);
    string request = "GET " + download_source.file_path + "/" +
      download_source.file_name + " HTTP/1.1\r\nRange: bytes=" +
      current_pos + "-" +
      to_string(connection.chunk.end_pos) + "\r\n" +  "Host:" +
      host_name +	":" + port + "\r\n\r\n";
    if(!send_data(connection, request.c_str(), request.length()))
      connections[index].status = OperationStatus::SOCKET_SEND_ERROR;
  }
}

void HttpDownloader::send_request(Connection& connection)
{
  const string port = to_string(download_source.port);
  const string host_name = download_source.host_name;
  string current_pos = to_string(connection.chunk.current_pos);

  string request = "GET " + download_source.file_path + "/" +
    download_source.file_name + " HTTP/1.1\r\nRange: bytes=" +
    current_pos + "-" +
    to_string(connection.chunk.end_pos) + "\r\n" +  "Host:" +
    host_name +	":" + port + "\r\n\r\n";

  if(!send_data(connection, request.c_str(), request.length()))
    connection.status = OperationStatus::SOCKET_SEND_ERROR;
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
    if (connections[index].status == OperationStatus::NOT_STARTED
        && recvd_bytes > 0) {
      buffer_offset = get_header_delimiter_position(buffer) + 4;
      recvd_bytes -= buffer_offset;
      connections[index].status = OperationStatus::DOWNLOADING;
    }
    else
      buffer_offset = 0;
  }

  return recvd_bytes;
}
