#include "http_downloader.h"

#include <regex>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>

#include "node.h"

using namespace std;

HttpDownloader::HttpDownloader(const struct DownloadSource& download_source,
                               const std::vector<int>& socket_descriptors)
  : Downloader(download_source, socket_descriptors)
{
  for (size_t index = 0; index < socket_descriptors.size(); ++index)
    connections[index].sock_desc = socket_descriptors[index];
}

HttpDownloader::HttpDownloader(const struct DownloadSource& download_source,
                               std::vector<int>& socket_descriptors,
                               std::unique_ptr<Writer> writer,
                               ChunksCollection& chunks_collection,
                               long int timeout)
  : Downloader(download_source, socket_descriptors, move(writer),
               chunks_collection, timeout)
{
  for (auto chunk : chunks_collection) {
    connections[chunk.first].chunk = chunk.second;
    connections[chunk.first].sock_desc = socket_descriptors.at(chunk.first);
    connections[chunk.first].status = OperationStatus::NOT_STARTED;
  }
}

void HttpDownloader::downloader_trd()
{
  fd_set readfds;

  static constexpr size_t kBufferLen = 40000;
  char recv_buffer[kBufferLen];

  // TODO it should resend request in case of error
  for (size_t index = 0; index < connections.size(); ++index)
    send_request(index);

  const size_t kFileSize = writer->get_file_size();
  while (writer->get_total_written_bytes() < kFileSize) {

    int max_fd = 0;
    FD_ZERO(&readfds);
    for (size_t index = 0; index < connections.size(); ++index) {
      int sock_desc = connections[index].sock_desc;
      FD_SET(sock_desc, &readfds);
      max_fd = (max_fd < sock_desc) ? sock_desc : max_fd;
    }

    int sel_retval = select(max_fd+1, &readfds, NULL, NULL, &timeout);

    if (sel_retval == -1)
      cerr << "Select error occured." << endl;
    else if (sel_retval > 0) {
      for (size_t index = 0; index < connections.size(); ++index) {
        int sock_desc = connections[index].sock_desc;
        if (FD_ISSET(sock_desc, &readfds)) {  // read from the socket
          size_t recvd_bytes = 0;
          receive_data(connections[index], recv_buffer,  recvd_bytes,
                       kBufferLen);

          // Skip the HTTP header
          size_t header_offset = 0;
          if (connections[index].status == OperationStatus::NOT_STARTED) {
            header_offset = get_header_delimiter_position(recv_buffer) + 4;
            recvd_bytes -= header_offset;
          }

          writer->write(recv_buffer+header_offset, recvd_bytes,
                        connections[index].chunk.current_pos, index);
          connections[index].chunk.current_pos += recvd_bytes;
          connections[index].status = OperationStatus::DOWNLOADING;
        }
      }   // End of for loop
    }   // End of else if
    else {    // Timeout
      break;    // Break while loop
    }   // End of else for timeout
  }   // End of while loop
}   // End of downloader_trd()

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

  // Use GET instead of HEAD, because some servers doesn't support HEAD
  //  command.
  string request =
    "GET " + download_source.file_path_on_server + " HTTP/1.1\r\nHost: " +
    download_source.host_name + ":" + to_string(download_source.port) +
    "\r\n\r\n";

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
      // If responst not equal to 200,OK
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

void HttpDownloader::send_request(size_t index)
{
  string request = "GET " + download_source.file_path_on_server +
    " " +	"HTTP/1.1\r\nRange: bytes=" +
    to_string(chunks_collection[index].current_pos) + "-" +
    to_string(chunks_collection[index].end_pos) + "\r\n" +  "Host:" +
    download_source.host_name +	":" + to_string(download_source.port) +
    "\r\n\r\n";

  if(!send_data(connections[index], request.c_str(), request.length()))
    connections[index].status = OperationStatus::SOCKET_SEND_ERROR;
}
