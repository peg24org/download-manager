#include "secure_transceiver.h"

#include <iostream>
using namespace std;

ssize_t SecureTransceiver::receive(SSL* ssl, char* buffer, const size_t len)
{
  ssize_t recvd_bytes = 0;
  recvd_bytes = SSL_read(ssl, buffer, len);
  if (recvd_bytes <= 0) {
    //cerr << "error SSL_read" << endl;
    // TODO: SSL_get_error()
  }

  return recvd_bytes;
}

bool SecureTransceiver::send(BIO* bio, const char* buffer, const size_t len)
{
  bool result = true;
  int64_t sent_bytes = 0;

  while (static_cast<size_t>(sent_bytes) < len) {
    int64_t temp_sent_bytes = BIO_write(bio, buffer, len);
    if (temp_sent_bytes > 0)
      sent_bytes += temp_sent_bytes;
    else if(temp_sent_bytes == -2) {
      cerr << "Operation not implemented in the specific BIO type." << endl;
      result = false;
      break;
    }
  }

  BIO_flush(bio);

  return result;
}

