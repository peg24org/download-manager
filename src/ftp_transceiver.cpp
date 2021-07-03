#include "ftp_transceiver.h"

#include <array>
#include <sstream>
#include <cstring>

using namespace std;

bool FtpTransceiver::send_init_commands(SocketOps* sock_ops)
{
  bool result = true;
  constexpr char USERNAME[] = "anonymous";
  constexpr char PASSWORD[] = "anonymous";

  const array<Buffer, 5> init_commands = {
    Buffer(string("")),
    Buffer(string("USER ") + USERNAME + "\r\n"),
    Buffer(string("PASS ") + PASSWORD + "\r\n"),
    Buffer("TYPE I\r\n"),
    Buffer("PWD\r\n")
  };

  string reply;
  for (const auto command : init_commands) {
    if (!send(command, sock_ops))
      cerr << "Ftp command sending error: " << reply << endl;
    else {
      Buffer recv_buffer;
      while (true) {
        if (!receive(recv_buffer, sock_ops)) {
          cerr << "Ftp receive error" << endl;
          result = false;
          break;
        }

        // RFC9559:
        // A reply is defined to contain the 3-digit code, followed by space.
        // string response_code_str(recv_buffer, 3);

        if (strstr(recv_buffer, "\r\n") != nullptr)
          break;
      }
    }
  }
  return result;
}

bool FtpTransceiver::receive(Buffer& buffer, Connection& connection)
{
  bool result = true;
  SocketOps* sock_ops = connection.socket_ops.get();
  result = receive(buffer, sock_ops);

  return result;
}

bool FtpTransceiver::send(const Buffer& buffer, Connection& connection)
{
  bool result = false;
  int sock_desc = connection.socket_ops->get_socket_descriptor();
  const char* raw_buffer = const_cast<Buffer&>(buffer);
  result = plain_transceiver.send(raw_buffer, buffer.length(), sock_desc);

  return result;
}

bool FtpTransceiver::receive(Buffer& buffer, SocketOps* sock_ops)
{
  ssize_t recvd_bytes = 0;
  recvd_bytes = plain_transceiver.receive(buffer,
                                          buffer.capacity(),
                                          sock_ops->get_socket_descriptor());
  buffer.set_length(recvd_bytes);

  if (recvd_bytes >= 0)
    return true;
  else
    return false;
}

bool FtpTransceiver::send(const Buffer& buffer, SocketOps* sock_ops)
{
  bool result = false;
  int sock_desc = sock_ops->get_socket_descriptor();
  result = plain_transceiver.send(const_cast<Buffer&>(buffer), buffer.length(),
                                  sock_desc);

  return result;
}

