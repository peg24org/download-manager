#ifndef _FTP_TRANSCEIVER_H
#define _FTP_TRANSCEIVER_H

#include "socket_ops.h"
#include "transceiver.h"
#include "plain_transceiver.h"

class FtpTransceiver : public Transceiver
{
  public:
    virtual bool receive(Buffer& buffer, Connection& connection) override;
    virtual bool receive(Buffer& buffer, SocketOps* sock_ops) override;
    virtual bool send(const Buffer& buffer, Connection& connection) override;
    virtual bool send(const Buffer& buffer, SocketOps* sock_ops) override;
    bool send_init_commands(SocketOps* sock_ops);

  private:
    PlainTransciever plain_transceiver;
};

#endif

