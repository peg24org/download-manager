#ifndef _PLAIN_TRANSCEIVER_H
#define _PLAIN_TRANSCEIVER_H

#include <sys/types.h>

class PlainTransciever
{
  public:
  ssize_t receive(char* buffer, size_t len, int socket_desc);
  bool send(const char* buffer, size_t len, int socket_desc);
};

#endif

