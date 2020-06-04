#include <arpa/inet.h>
#include <sys/socket.h> 

#include <mutex>
#include <regex>
#include <cstdlib>
#include <cassert>

#include "node.h"
#include "downloader.h"

const string
  Downloader::HTTP_HEADER = "(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)";

void Downloader::call_node_status_changed(int received_bytes,
    StatusStruct status)
{
}

void Downloader::set_index(int value)
{
  index = value;
}

int Downloader::get_index()
{
  return index;
}

size_t Downloader::get_current_pos() const
{
  return pos;
}

size_t Downloader::get_trd_len()
{
  return trd_len;
}

StatusStruct Downloader::get_status() const
{
  return status;
}

void Downloader::run()
{
  downloader_trd();
}

void Downloader::write_to_file(size_t pos, size_t len, char* buf)
{
  const lock_guard<mutex> lock(node_data->node_mutex);

  file_io.write(buf, pos, len);
  logger.write(index, pos + len);
}

void Downloader::write_start_pos_log(size_t start_pos)
{
  node_data->log_buffer_str += "s" + to_string(index) + "\t" +
    to_string(start_pos)+"\n";
  rewind(node_data->log_fp);
  fwrite(node_data->log_buffer_str.c_str(), 1,
      node_data->log_buffer_str.length(), node_data->log_fp);
}

bool Downloader::regex_search_string(const string& input,
    const string& pattern, string& output, int pos_of_pattern)
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

bool Downloader::connection_init()
{
  // Check if connected before
  if(sockfd)
    return true;

  struct sockaddr_in dest_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) {
    set_status(OperationStatus::SOCKFD_ERROR, errno);
    return false;
  }
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(addr_data.port);
  dest_addr.sin_addr.s_addr = inet_addr(addr_data.ip.c_str());
  memset(&(dest_addr.sin_zero),'\0', sizeof(dest_addr.sin_zero));
  if(connect(sockfd, (struct sockaddr *)&dest_addr,
        sizeof(struct sockaddr)) < 0) {
    set_status(OperationStatus::SOCKET_CONNECT_FUNCTION_ERROR, errno);
    return false;
  }

  // Set timeout
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_interval,
      sizeof(timeout_interval));
  return true;
}

bool Downloader::socket_send(const char* buffer, size_t len)
{
  size_t sent_bytes = 0;
  size_t tmp_sent_bytes = 0;

  while (sent_bytes < len) {
    if ((tmp_sent_bytes = send(sockfd, buffer, len, 0)) > 0)
      sent_bytes += tmp_sent_bytes;
    else {
      set_status(OperationStatus::SOCKET_SEND_FUNCTION_ERROR, errno);
      return false;
    }
  }
  return true;
}

bool Downloader::check_error(int len) const
{
  if (len < 0) {
    perror("ERROR ");
    exit(1);
  }
  return true;
}

bool Downloader::socket_receive(char* buffer, size_t& received_len,
    size_t buffer_capacity)
{
  ssize_t received_bytes = recv(sockfd, buffer, buffer_capacity, 0);
  if (received_bytes < 0) {
    set_status(OperationStatus::SOCKET_RECV_FUNCTION_ERROR, errno);
    return false;
  }
  else
    received_len = received_bytes;
  return true;
}

void Downloader::set_status(OperationStatus operation_status, int error_type)
{
  status.operation_status = operation_status;
  status.error_value = error_type;
}
