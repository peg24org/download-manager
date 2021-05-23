#ifndef _TRANSCEIVER_H
#define _TRANSCEIVER_H

#include "buffer.h"
#include "socket_ops.h"
#include "connection.h"

class Transceiver
{
  public:
  bool send(const Buffer& buffer, SocketOps* socket_ops);
  bool send(const char* buffer, size_t length, SocketOps* socket_ops);
  virtual bool receive(Buffer& buffer, Connection& connection);
  bool receive(Buffer& buffer, SocketOps* socket_ops);
  ssize_t receive(char* buffer, size_t length, SocketOps* socket_ops);
};

#endif

