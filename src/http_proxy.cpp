#include "http_proxy.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <cstring>

using namespace std;

HttpProxy::HttpProxy(const string& dest_host_name, uint16_t dest_port)
  : kHttpRequest("CONNECT " + dest_host_name + ":" + to_string(dest_port) +
                 " HTTP/1.0\r\n\r\n")
{
}

int HttpProxy::connect(int socket_desc)
{
  size_t sent_bytes = 0;
  size_t tmp_sent_bytes = 0;

  while (sent_bytes < kHttpRequest.length()) {
    tmp_sent_bytes = send(socket_desc, kHttpRequest.c_str(),
                          kHttpRequest.length(), 0);
    if (tmp_sent_bytes > 0)
      sent_bytes += tmp_sent_bytes;
    //TODO : handle else
  }

  constexpr uint16_t kTempLen = 10000;
  char temp_buffer[kTempLen];
  string buffer;
  while (true) {
    int64_t recv_len = recv(socket_desc, temp_buffer, kTempLen, 0);
    buffer += string(temp_buffer, recv_len);
    if (buffer.find("\r\n\r\n") != string::npos)
      break;
  }

  return 0;
}
