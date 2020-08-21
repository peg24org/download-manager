#ifndef _HTTPS_DOWNLOADER_H
#define _HTTPS_DOWNLOADER_H

#include <openssl/ssl.h>

#include "file_io.h"
#include "http_downloader.h"

class HttpsDownloader : public HttpDownloader {
  public:
  ~HttpsDownloader();

  HttpsDownloader(const struct DownloadSource& download_source,
                  const std::vector<int>& socket_descriptors);

  HttpsDownloader(const struct DownloadSource& download_source,
                  std::vector<int>& socket_descriptors,
                  std::unique_ptr<Writer> writer,
                  ChunksCollection& chunks_collection);

  private:
  void ssl_init_sockets();
  bool receive_data(Connection& connection, char* buffer, size_t& received_len,
                    size_t buffer_capacity) override;
  bool send_data(Connection& connection, const char* buffer,
                 size_t len) override;

  void verify_the_certificate(SSL *ssl);
  SSL* get_ssl(BIO* bio);
};

#endif
