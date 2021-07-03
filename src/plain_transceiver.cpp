#include "plain_transceiver.h"

#include <sys/socket.h>

ssize_t PlainTransciever::receive(char* buffer, size_t len, int socket_desc)
{
  ssize_t recvd_len = recv(socket_desc, buffer, len, 0);

  return recvd_len;
}

bool PlainTransciever::send(const char* buffer, size_t length, int socket_desc)
{
  bool result = true;
  size_t sent_bytes = 0;
  size_t tmp_sent_bytes = 0;

  while (sent_bytes < length) {
    tmp_sent_bytes = ::send(socket_desc, buffer+sent_bytes, length, 0);
    if (tmp_sent_bytes >= 0)
      sent_bytes += tmp_sent_bytes;
    else {
      result = false;
      break;
    }
  }

  return result;
}

