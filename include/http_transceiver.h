#ifndef _HTTP_TRANSCEIVER_H
#define _HTTP_TRANSCEIVER_H

#include "connection.h"
#include "transceiver.h"
#include "plain_transceiver.h"

class HttpTransceiver : public Transceiver
{
  public:
    virtual bool receive(Buffer& buffer, Connection& connection) override;
    virtual bool receive(Buffer& buffer, SocketOps* sock_ops) override;
    virtual bool send(const Buffer& buffer, Connection& connection) override;
    virtual bool send(Buffer& buffer, SocketOps* sock_ops) override;

  protected:
    static constexpr char kHeaderTerminator[] = "\r\n\r\n";
 
    ssize_t get_header_terminator_pos(const char* buffer, size_t len) const;

  private:
    PlainTransciever plain_transceiver;
};

#endif

