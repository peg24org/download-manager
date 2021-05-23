#include "connection.h"

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <cstring>
#include <sstream>

#include "buffer.h"

using namespace std;

Connection::Connection(const string& host_name, uint16_t port) :
  host_name(host_name),
  port(port),
  proxy_host_name(""),
  proxy_port(0),
  socket_descriptor(0)
{
}

void Connection::set_proxy(const string& host_name, uint16_t port)
{
  proxy_host_name = host_name;
  proxy_port = port;
}

bool Connection::connect()
{
  bool result = false;
  if (proxy_port != 0 && !proxy_host_name.empty()) {
    string ip = get_dest_ip(proxy_host_name);
    socket_connect(ip);
  }
  else {
    string ip = get_dest_ip(host_name);
    socket_connect(ip);
    result = true;
  }

  return result;
}

bool Connection::socket_connect(const string& ip)
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

string Connection::get_dest_ip(const string& dest_host_name) const
{
  struct hostent* server;
  server = gethostbyname(dest_host_name.c_str());
  if (!server)
    throw runtime_error("cannot get ip.");

  return string(inet_ntoa(*((struct in_addr*) server->h_addr)));
}

bool Connection::connect_to_proxy()
{
  bool result = true;
  stringstream http_request_stream;
  http_request_stream << "CONNECT" << " " << host_name << ":" << port << " "
                      << "HTTP/1.0\r\n\r\n";
  const string kHttpRequest = http_request_stream.str();
  size_t sent_bytes = 0;

  while (sent_bytes < kHttpRequest.length()) {
    ssize_t tmp_sent_bytes = send(socket_descriptor, kHttpRequest.c_str()+sent_bytes,
                                  kHttpRequest.length(), 0);
    if (tmp_sent_bytes > 0)
      sent_bytes += tmp_sent_bytes;
    else if (tmp_sent_bytes < 0) {
      result = false;
      break;
    }
  }

  Buffer temp_buffer;
  string buffer;
  while (true) {
    int64_t recv_len = recv(socket_descriptor, temp_buffer, temp_buffer.capacity(), 0);
    buffer += string(temp_buffer, recv_len);
    if (buffer.find("\r\n\r\n") != string::npos) {
      break;
    }
  }

  return result;
}
