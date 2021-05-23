#include "http_transceiver.h"

#include <cstring>
#include <iostream>

using namespace std;

bool HttpTransceiver::receive(Buffer& buffer, Connection& connection)
{
  bool result;
  if (!connection.header_skipped) {
    result = receive(buffer, connection.socket_ops.get(), true);
    connection.header_skipped = true;
  }
  else {
    result = receive(buffer, connection.socket_ops.get(), false);
  }

  return result;
}

bool HttpTransceiver::receive(Buffer& buffer, SocketOps* socket_ops,
                              bool skip_header)
{
  bool result = true;

  if (!skip_header) {
      result = Transceiver::receive(buffer, socket_ops);
      return result;
  }

  while (skip_header) {
    Buffer temp_buffer;
    result = Transceiver::receive(temp_buffer, socket_ops);
    if (result == false) {
      cerr << "RECV ERROR" << endl;
      break;
    }
    const ssize_t header_pos = get_header_terminator_pos(temp_buffer,
                                                         temp_buffer.length());
    if (header_pos > -1) {
      buffer = temp_buffer;
      size_t pos = header_pos;
      memcpy(buffer, temp_buffer + pos, temp_buffer.length() - pos);
      buffer.set_length(temp_buffer.length() - pos);
      break;
    }
  }

  return result;
}

ssize_t HttpTransceiver::get_header_terminator_pos(const char* buffer,
                                                   size_t len) const
{
  const char* head_position = strstr(buffer, kHeaderTerminator);
  if (head_position != nullptr)
    //return head_position - buffer + strlen(kHeaderTerminator);
    return head_position - buffer + 4;
  else
    return -1;
}

