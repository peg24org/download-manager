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

void Downloader::call_node_status_changed(int received_bytes, int err_flag)
{
  static_cast<Node*>(node_data->node)->on_get_status(&addr_data, index, trd_len,
      received_bytes, 0);
}

void Downloader::set_index(int value)
{
  index = value;
}

int Downloader::get_index()
{
  return index;
}

size_t Downloader::get_trd_len()
{
  return trd_len;
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

void Downloader::write_log_file(size_t pos)
{

  regex e("(.|\\s)*(p"+to_string(index)+"\t\\d+\n)(.|\\s)*");
  if(node_data->log_buffer_str.length()<1)
    node_data->log_buffer_str = "p" + to_string(index) + "\t" +
      to_string(pos) + "\n";
  else{
    if (regex_match(node_data->log_buffer_str, e)) {
      regex re("p"+to_string(index)+"\\t\\d+\\n");
      node_data->log_buffer_str = regex_replace(node_data->log_buffer_str,
          re, "p" + to_string(index) + "\t" + to_string(pos) + "\n");
    }
    else
      node_data->log_buffer_str += "p" + to_string(index) + "\t" +
        to_string(pos) + "\n";
  }

  rewind(node_data->log_fp);
  fwrite(node_data->log_buffer_str.c_str(), 1,
      node_data->log_buffer_str.length(), node_data->log_fp);
  if (!is_start_pos_written && !node_data->resuming){
    write_start_pos_log(pos);
    is_start_pos_written = true;
  }
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

bool Downloader::socket_send(const char* buffer, size_t len)
{
  size_t sent_bytes = 0;
  size_t tmp_sent_bytes = 0;

  while (sent_bytes < len) {
    if ((tmp_sent_bytes = send(sockfd, buffer, len, 0)) > 0)
      sent_bytes += tmp_sent_bytes;
    else
      check_error(tmp_sent_bytes);
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
  return (received_len =
      recv(sockfd, buffer, buffer_capacity, 0)) > 0 ? true : false;
}
