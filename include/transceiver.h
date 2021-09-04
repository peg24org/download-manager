#ifndef _TRANSCEIVER_H
#define _TRANSCEIVER_H

#include "buffer.h"
#include "socket_ops.h"
#include "connection.h"

class Transceiver
{
  public:
  virtual bool receive(Buffer& buffer, Connection& connection) = 0;
  virtual bool receive(Buffer& buffer, SocketOps* sock_ops) = 0;
  virtual bool receive(Buffer& buffer, SocketOps* sock_ops,
                       bool& header_skipped);
  virtual bool send(const Buffer& buffer, Connection& socket_ops) = 0;
  virtual bool send(const Buffer& buffer, SocketOps* sock_ops) = 0;
};

#endif

