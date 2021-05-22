#include "socket_ops.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>

using namespace std;

SocketOps::~SocketOps()
{
  close(socket_descriptor);
}

SocketOps::SocketOps(const string& ip, uint16_t port)
  : socket_descriptor(0), ip(ip), port(port)
{
}

bool SocketOps::connect()
{
  bool result = false;
  socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_descriptor > 0) {
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    memset(&(dest_addr.sin_zero), '\0', sizeof(dest_addr.sin_zero));

    socklen_t addr_len = sizeof(struct sockaddr);
    struct sockaddr* address = reinterpret_cast<struct sockaddr*>(&dest_addr);
    if (::connect(socket_descriptor, address, addr_len) == 0)
      result = true;
  }

  return result;
}

bool SocketOps::disconnect()
{
  bool result = false;

  if (close(socket_descriptor) == 0)
    result = true;

  return result;
}

int SocketOps::get_socket_descriptor() const noexcept
{
  return socket_descriptor;
}


void SocketOps::set_http_proxy(const std::string& host, uint16_t port)
{
  http_proxy_host = host;
  proxy_port = port;
}

