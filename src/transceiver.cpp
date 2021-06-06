//#include "transceiver.h"
//
//#include <sys/socket.h>
//
//bool Transceiver::send(const Buffer& buffer, SocketOps* socket_ops)
//{
//  return send(const_cast<Buffer&>(buffer), buffer.length(), socket_ops);
//}
//
//bool Transceiver::send(const char* buffer, size_t length, SocketOps* socket_ops)
//{
//  bool result = true;
//  size_t sent_bytes = 0;
//  size_t tmp_sent_bytes = 0;
//  int socket = socket_ops->get_socket_descriptor();
//
//  while (sent_bytes < length) {
//    tmp_sent_bytes = ::send(socket, buffer+sent_bytes, length, 0);
//    if (tmp_sent_bytes >= 0)
//      sent_bytes += tmp_sent_bytes;
//    else {
//      result = false;
//      break;
//    }
//  }
//
//  return result;
//}
//
//bool Transceiver::receive(Buffer& buffer, SocketOps* socket_ops)
//{
//  ssize_t recvd_len = receive(buffer + buffer.length(), buffer.capacity(),
//                              socket_ops);
//
//  if (recvd_len < 0)
//    return false;
//
//  buffer.set_length(recvd_len);
//
//  return true;
//}
//
//bool Transceiver::receive(Buffer& buffer, Connection& connection)
//{
//  return receive(buffer, connection.socket_ops.get());
//}
//
//ssize_t Transceiver::receive(char* buffer, size_t length, SocketOps* socket_ops)
//{
//  int socket = socket_ops->get_socket_descriptor();
//  return recv(socket, buffer, length, 0);
//}
//
