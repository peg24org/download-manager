#ifndef _HTTP_TRANSCEIVER_H
#define _HTTP_TRANSCEIVER_H

#include "transceiver.h"

class HttpTransceiver : public Transceiver
{
  public:
    bool receive(Buffer& buffer, SocketOps* socket_ops, bool skip_header);

  private:
    static constexpr char kHeaderTerminator[] = "\r\n\r\n";
 
    ssize_t get_header_terminator_pos(const char* buffer, size_t len) const;
};

#endif

