#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/err.h>

#include <iostream>

#include "https_downloader.h"

HttpsDownloader::~HttpsDownloader()
{
  disconnect();
}

void HttpsDownloader::disconnect()
{
  if (ssl != nullptr){
    SSL_free(ssl);
    ssl = nullptr;
  }
  if (sockfd != 0){
    close(sockfd);
    sockfd = 0;
  }
}

bool HttpsDownloader::socket_send(const char* buffer, size_t len)
{
  size_t sent_bytes = 0;
  size_t tmp_sent_bytes = 0;
  while (sent_bytes < len){
    if ((tmp_sent_bytes = SSL_write(ssl, buffer, len)) > 0)
      sent_bytes += tmp_sent_bytes;
    else {
      check_error(tmp_sent_bytes);
      cerr << " Https Err " << endl;
      return false;
    }
  }
  return true;
}

bool HttpsDownloader::check_error(int len) const
{
  if (len < 0) {
    int error_number = SSL_get_error(ssl, len);
    switch (error_number) {
      case SSL_ERROR_WANT_WRITE:
      case SSL_ERROR_WANT_READ:
        return true;
      case SSL_ERROR_ZERO_RETURN:
      case SSL_ERROR_SYSCALL:
      case SSL_ERROR_SSL:
      default:
        return false;
    }
  }
  return true;
}

bool HttpsDownloader::http_connect()
{
  connection_init();

  SSL_library_init();
  SSLeay_add_ssl_algorithms();
  SSL_load_error_strings();
  const SSL_METHOD *meth = TLS_client_method();
  SSL_CTX *ctx = SSL_CTX_new (meth);

  ssl = SSL_new(ctx);

  if (!ssl)
    cerr << "Error creating SSL." << endl;

  SSL_set_fd(ssl, sockfd);
  int error_number = SSL_connect(ssl);

  if (error_number <= 0){
    set_status(OperationStatus::SSL_ERROR, error_number);
    return false;
  }
  return true;
}
