#include "http_transceiver.h"

#include <cstring>
#include <iostream>

using namespace std;

bool HttpTransceiver::receive(Buffer& buffer, Connection& connection)
{
  SocketOps* sock_ops = connection.socket_ops.get();
  const bool result = receive(buffer, sock_ops);
  return result;
}

bool HttpTransceiver::send(const Buffer& buffer, Connection& connection)
{
  bool result = false;
  int sock_desc = connection.socket_ops->get_socket_descriptor();
  const char* raw_buffer = const_cast<Buffer&>(buffer);
  result = plain_transceiver.send(raw_buffer, buffer.length(), sock_desc);

  return result;
}

ssize_t HttpTransceiver::get_header_terminator_pos(const char* buffer,
                                                   size_t len) const
{
  const char* head_position = strstr(buffer, kHeaderTerminator);
  if (head_position != nullptr)
    // strlen(kHeaderTerminator) = 4
    return head_position - buffer + 4;
  else
    return -1;
}

bool HttpTransceiver::receive(Buffer& buffer, SocketOps* sock_ops)
{
  ssize_t recvd_bytes = 0;
  recvd_bytes = plain_transceiver.receive(buffer, buffer.capacity(),
                                          sock_ops->get_socket_descriptor());
  buffer.set_length(recvd_bytes);

  if (recvd_bytes >= 0)
    return true;
  else
    return false;
}

bool HttpTransceiver::send(const Buffer& buffer, SocketOps* sock_ops)
{
  bool result = false;
  int sock_desc = sock_ops->get_socket_descriptor();
  result = plain_transceiver.send(const_cast<Buffer&>(buffer), buffer.length(),
                                  sock_desc);

  return result;
}

