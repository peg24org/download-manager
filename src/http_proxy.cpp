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
  size_t len;

  while (sent_bytes < kHttpRequest.length()) {
    tmp_sent_bytes = send(socket_desc, kHttpRequest.c_str(),
                          kHttpRequest.length(), 0);
    if (tmp_sent_bytes > 0)
      sent_bytes += tmp_sent_bytes;
    //TODO : handle else
  }

  size_t received_len = 0;
  char recv_buffer[10000];
  while (true) {
    int64_t recv_len = recv(socket_desc, recv_buffer, 10000, 0);
    if (recv_len >= 0)
      received_len = recv_len;
    if (strstr(recv_buffer, "\r\n\r\n"))
      break;
    // TODO: handle else
  }

  return 0;
}
