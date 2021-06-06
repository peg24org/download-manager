#include "https_transceiver.h"

#include <iostream>

#include "https_socket_ops.h"

using namespace std;

bool HttpsTransceiver::receive(Buffer& buffer, SocketOps* sock_ops)
{
  bool result = false;
  ssize_t recvd_bytes = 0;
  SSL* ssl = static_cast<HttpsSocketOps*>(sock_ops)->get_ssl();
  recvd_bytes = secure_transceiver.receive(ssl, buffer, buffer.capacity());
  string strbuf = string(buffer, recvd_bytes);

  if (recvd_bytes >= 0) {
    buffer.set_length(recvd_bytes);
    result = true;
  }
  else
    result = false;

  cerr << "https receiveçççççççççççççççççççççççççççç" <<
    string(buffer, buffer.length()) << endl << endl;

  return result;
}

bool HttpsTransceiver::send(Buffer& buffer, SocketOps* sock_ops)
{
  cerr << "REQUEST in https send:" << string(buffer, buffer.length()) << endl;
  bool result = true;
  BIO* bio = dynamic_cast<HttpsSocketOps*>(sock_ops)->get_bio();
  secure_transceiver.send(bio, buffer, buffer.length());

  return result;
}

