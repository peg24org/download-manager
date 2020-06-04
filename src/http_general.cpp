#include <regex>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <iostream>

#include "node.h"
#include "definitions.h"
#include "http_general.h"

void HttpGeneral::downloader_trd()
{
  set_status(OperationStatus::NOT_STARTED);

  string request = "GET " + addr_data.file_path_on_server + " " +
    "HTTP/1.1\r\nRange: bytes=" + to_string(pos) + "-" +
    to_string(pos + trd_len) + "\r\n" +  "Host:" + addr_data.host_name +
    ":" + to_string(addr_data.port) + "\r\n\r\n";

  bool send_request_result = false;
  if(http_connect())
    send_request_result = socket_send(request.c_str(), request.length());

  unique_ptr<char[]> buffer_unique_ptr =
    make_unique<char[]>(CHUNK_SIZE * sizeof(char));
  char* buffer = buffer_unique_ptr.get();

  char* header_delimiter_pos = 0;
  size_t temp_received_bytes = 0;
  //size_t total_received_bytes = 0;

  while(send_request_result && (total_received_bytes < trd_len)) {
    size_t bytes = 0;
    if (!socket_receive(buffer, bytes, CHUNK_SIZE)) {
      break;
    }
    if(!header_delimiter_pos) {
      header_delimiter_pos = get_header_delimiter_position(buffer);
      if (header_delimiter_pos){
        // [\r\n\r\n]=4 Bytes
        const size_t header_delimiter = (header_delimiter_pos - buffer) + 4;
        temp_received_bytes = bytes - header_delimiter;
        write_to_file(pos, temp_received_bytes, buffer
            + header_delimiter);//n=length
        pos += temp_received_bytes;
        total_received_bytes += temp_received_bytes;
        set_status(OperationStatus::DOWNLOADING);
      }
      else {
        buffer += bytes;
        header_delimiter_pos = NULL;
        continue;
      }
    }
    else {
      write_to_file(pos, bytes, buffer);//n=length
      pos += bytes;
      total_received_bytes += bytes;
      temp_received_bytes = bytes;
      set_status(OperationStatus::DOWNLOADING);
    }
    call_node_status_changed(temp_received_bytes, get_status());
  }   // End of while loop

  if(total_received_bytes == trd_len || total_received_bytes == trd_len+1)
      set_status(OperationStatus::FINISHED);

  call_node_status_changed(temp_received_bytes, get_status());
}

size_t HttpGeneral::get_size()
{
  string size_string;
  if (regex_search_string(receive_header, "(Content-Length: )(\\d+)",
        size_string))
    return  strtoul(static_cast<const char*>(size_string.c_str()), nullptr, 0);
  return 0;
}

bool HttpGeneral::http_connect()
{
  return connection_init();
}

bool HttpGeneral::check_redirection(string& redirecting)
{
  if (regex_search_string(receive_header, HTTP_HEADER, redirecting))
  {
    // Check redirection
    if (regex_search_string(receive_header, "(Location: )(.+)",redirecting))
      return true;
    return false;
  }
  return false;
}

int HttpGeneral::check_link(string& redirected_url, size_t& file_size)
{
  int redirect_status = 0;

  // Use GET instead of HEAD, because some servers doesn't support HEAD command.
  string request =
    "GET " + addr_data.file_path_on_server + " HTTP/1.1\r\nHost:" +
    addr_data.host_name + "\r\n\r\n";
  if(!http_connect())
    return -1;

  receive_header.resize(MAX_HTTP_HEADER_LENGTH);

  if (!socket_send(request.c_str(), request.length()))
    return -1;    // Report error

  size_t len = 0;
  unique_ptr<char[]> buffer = make_unique<char[]>(CHUNK_SIZE * sizeof(char));

  while (true) {
    size_t number_of_bytes;
    if (!socket_receive(const_cast<char*>(receive_header.data()),
          number_of_bytes, MAX_HTTP_HEADER_LENGTH)) {
      return -1;
    }
    len += number_of_bytes;
    if (receive_header.find("\r\n\r\n") != string::npos)
      break;
  }

  if (check_redirection(redirected_url))
    // Link is redirected
    return 1;
  else {
    file_size = get_size();
    // Link is not redirected
    redirect_status = 0;
  }

  disconnect();
  return redirect_status;
}

char* HttpGeneral::get_header_delimiter_position(const char* buffer)
{
  char* delimiter_pos = const_cast<char*>(strstr(buffer,"\r\n\r\n"));
  if(delimiter_pos) {
    string recv_buffer = string(buffer);
    smatch m;
    regex e("(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)");
    bool found = regex_search(recv_buffer, m, e);
    if(found) {
      if(stoi(m[2].str())/100 != 2) {
        set_status(OperationStatus::HTTP_ERROR, stoi(m[2].str()));
        delimiter_pos = NULL;
      }
    }
    else {
      set_status(OperationStatus::RESPONSE_ERROR);
      delimiter_pos = NULL;
    }
  }
  return delimiter_pos;
}
