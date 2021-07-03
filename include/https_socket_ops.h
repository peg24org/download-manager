#ifndef _HTTPS_SOCKET_OPS_H
#define _HTTPS_SOCKET_OPS_H

#include <openssl/ssl.h>
#include <unistd.h>

#include "socket_ops.h"

// Socket operations for secure connection.
class HttpsSocketOps : public SocketOps
{
  public:
    HttpsSocketOps(const std::string& ip, uint16_t port,
                   const std::string& host);

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
    SSL* retrieve_ssl(BIO* bio);

    BIO* bio;
    SSL* ssl;
    SSL_CTX* ctx{nullptr};
    X509 *cert{nullptr};
    std::string host;
};

#endif
