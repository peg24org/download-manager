#ifndef _SOCKET_OPS_H
#define _SOCKET_OPS_H

#include <string>

#include "url_ops.h"

// Socket operations class.
class SocketOps
{
  public:
    /**
     *  The c-tor.
     *
     *  @param ip Ip address of remote host.
     *  @param port Port number of remote host.
     */
    SocketOps(const std::string& ip, uint16_t port);

    /**
     * Connects to 'ip' and 'port' specified in c-tor.
     * @return True if connecting is successful.
     */
     virtual bool connect();

    /**
     *  Disconnects socket from 'ip' and 'port'.
     *  @return True if disconnecting is successful.
     */
    virtual bool disconnect();

    /**
     *  Gets the socket descriptor for send/recv.
     *  @return The socket descriptor.
     */
    int get_socket_descriptor() const noexcept;

  protected:
    int socket_descriptor;
    const std::string ip;
    const uint16_t port;
};

#endif
