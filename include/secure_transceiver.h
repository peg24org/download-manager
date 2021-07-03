#ifndef _SECURE_TRANSCEIVER_H
#define _SECURE_TRANSCEIVER_H

#include <openssl/ssl.h>

class SecureTransceiver
{
  public:
  ssize_t receive(SSL* ssl, char* buffer, const size_t len);
  bool send(BIO* bio, const char* buffer, const size_t len);
};

#endif


