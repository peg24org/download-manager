#ifndef _HTTPS_SOCKET_OPS_H
#define _HTTPS_SOCKET_OPS_H

#include <openssl/ssl.h>
#include <unistd.h>

#include "url_ops.h"
#include "socket_ops.h"

// Socket operations for secure connection.
class HttpsSocketOps : public SocketOps
{
  public:
    /**
     *  The c-tor.
     *
     *  @see SocketOps::SocketOps()
     */
    using SocketOps::SocketOps;

    /**
     *  @see SocketOps::connect()
     */
    bool connect() override;

    /**
     *  @see SocketOps::disconnect()
     */
    bool disconnect() override;

    BIO* get_bio() const;
    SSL* get_ssl() const;

  private:
    SSL* get_ssl(BIO* bio); //const
    BIO* bio;
    SSL* ssl;
    int sock_desc{0};
    SSL_CTX* ctx{nullptr};
    X509 *cert{nullptr};
};

#endif
