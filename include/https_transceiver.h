#ifndef _HTTPS_TRANSCEIVER_H
#define _HTTPS_TRANSCEIVER_H

#include "http_transceiver.h"
#include "secure_transceiver.h"

class HttpsTransceiver : public HttpTransceiver
{
  public:
    bool send(const Buffer& buffer, SocketOps* sock_ops) final;
    bool receive(Buffer& buffer, SocketOps* sock_ops) final;

  private:
    SecureTransceiver secure_transceiver;
};

#endif

