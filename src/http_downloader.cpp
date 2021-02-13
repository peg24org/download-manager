#include "http_downloader.h"

#include <unistd.h>
#include <arpa/inet.h>

#include <regex>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <iostream>

#include "node.h"
#include <iostream>
using namespace std;

HttpDownloader::HttpDownloader(const struct DownloadSource& download_source)
  : Downloader(download_source)
{
  create_connection(true);
}

size_t HttpDownloader::get_size(string header)
{
  string size_string;
  if (regex_search_string(header, "(Content-Length: )(\\d+)", size_string))
    return  strtoul(static_cast<const char*>(size_string.c_str()), nullptr, 0);
  return 0;
}

bool HttpDownloader::check_file_availability(const string& header)
{
  bool result = false;
  string first_line = header.substr(0, header.find("\r\n"));
  // Check header
  string http_code_str;
  if (regex_search_string(first_line, "HTTP.* (\\d+) .*", http_code_str, 1)) {
    int http_code = stoi(http_code_str);
    if (http_code != 200) {
      cerr << "URL error, http response: " << http_code << endl;
      result = false;
    }
    else
      result = true;
  }

  return result;
}

bool HttpDownloader::check_redirection(string& redirecting,
                                       const string& header)
{
  bool result = false;
  // Check header
  if (regex_search_string(header, HTTP_HEADER)) {
    // Check redirection
    if (regex_search_string(header, "(Location: )(.+)", redirecting)) {
      result = true;
      // Check if redirecting to path instead of URL, then create new URL.
      if (!regex_search_string(header, "http.*://")) {
        const Protocol& protocol = download_source.protocol;
        string prefix = protocol == Protocol::HTTP ? "http://" : "https://";
        redirecting = prefix + download_source.host_name + redirecting;
      }
    }
  }
  return result;
}

HttpDownloader::HttpStatus HttpDownloader::get_http_status(const char* buffer,
                                                           size_t range)
{
  int status;
  size_t line_length = 0;
  const char* end_of_line = strstr(buffer, "\r\n");
  if (end_of_line == nullptr)
    return HttpStatus::NONE;

  line_length = end_of_line - buffer;
  string line(buffer, line_length);
  stringstream line_stream(line);;
  string http_version;
  line_stream >> http_version >> status;

  return static_cast<HttpStatus>(status);
}

int HttpDownloader::check_link(string& redirected_url, size_t& file_size)
{
  int redirect_status = 0;

  // Use GET instead of HEAD, because some servers doesn't support HEAD
  // command.
  Buffer request(1024);
  request << "GET "
          << download_source.file_path << download_source.file_name
          << " HTTP/1.1\r\n"
          << "User-Agent: no_name_yet!\r\n"
          << "Accept: */*\r\n"
          << "Accept: */*\r\n"
          << "Accept-Encoding: identity\r\n"
          << "Host: " << download_source.host_name << ':'
          << to_string(download_source.port) + "\r\n"
          << "Connection: Keep-Alive\r\n\r\n";

  Buffer header(MAX_HTTP_HEADER_LENGTH);
  if (!send_data(connections[0], request))
    return -1;

  string header_str;

  while (true) {
    if (!receive_data(connections[0], header.seek(header.length())))
      return -1;
    header_str = string(header, header.length());
    if (header_str.find("\r\n\r\n") != string::npos) {
      break;
    }
  }

  if (!check_file_availability(header_str))
    redirect_status = -1;
  else {
    if (check_redirection(redirected_url, header_str))    // Link is redirected
      redirect_status = 1;
    else {
      file_size = get_size(header_str);   // Link is not redirected
      redirect_status = 0;
    }
  }

  return redirect_status;
}

size_t HttpDownloader::get_header_terminator_pos(const string& buffer) const
{
  size_t terminator_pos = 0;
  const string kHeaderTerminator = "\r\n\r\n";
  terminator_pos = buffer.find(kHeaderTerminator);

  if (terminator_pos != string::npos)
    terminator_pos += 4;

  return terminator_pos;
}

bool HttpDownloader::send_requests()
{
  bool result = true;

  for (auto& [index, connection] : connections)
    if (!connection.request_sent)
      result &= send_request(connection);

  return result;
}

bool HttpDownloader::send_request(Connection& connection)
{
  const string port = to_string(download_source.port);
  const string host_name = download_source.host_name;
  string current_pos = to_string(connection.chunk.current);
  string end_pos = to_string(connection.chunk.end);

  Buffer request;
  request << "GET " << download_source.file_path << "/"
          << download_source.file_name << " HTTP/1.1\r\nRange: bytes="
          << current_pos << "-" << end_pos << "\r\n"
          << "User-Agent: no_name_yet!\r\n"
          << "Accept: */*\r\n"
          << "Accept-Encoding: identity\r\n"
          << "Host:" << host_name << ":" << port << "\r\n\r\n";

  if(!send_data(connection, request)) {
    connection.status = OperationStatus::SOCKET_SEND_ERROR;
    return false;
  }

  connection.request_sent = true;
  return true;
}

int HttpDownloader::set_descriptors()
{
  int max_fd = 0;
  FD_ZERO(&readfds);

  for (auto& [index, connection] : connections) {
    int sock_desc = connection.socket_ops->get_socket_descriptor();
    FD_SET(sock_desc, &readfds);
    max_fd = (max_fd < sock_desc) ? sock_desc : max_fd;
  }

  return max_fd;
}

void HttpDownloader::receive_from_connection(size_t index, Buffer& buffer)
{
  Connection& connection = connections[index];

  buffer.clear();
  int sock_desc = connection.socket_ops->get_socket_descriptor();

  if (FD_ISSET(sock_desc, &readfds)) {  // read from the socket
    receive_data(connection, buffer);

    // TODO: Add flag for first packet.
    if (!connection.header_skipped && buffer.length() > 0) {
      HttpDownloader::HttpStatus status = get_http_status(buffer,
                                                          buffer.length());
      if (status != HttpStatus::PARTIAL_CONTENT)
        buffer.clear();

      string& http_header = connection.temp_http_header;
      http_header += string(buffer, buffer.length());
      size_t header_terminator_pos = get_header_terminator_pos(http_header);

      if (header_terminator_pos == string::npos)
        buffer.clear();

      const char* kData = http_header.data();
      size_t payload_len = http_header.length() - header_terminator_pos;
      memcpy(buffer, kData + header_terminator_pos, payload_len);
      connection.header_skipped = true;
      buffer.set_length(payload_len);
      http_header.clear();
    }
  }
}
