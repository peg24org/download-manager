#ifndef _HTTPS_DOWNLOADER_H
#define _HTTPS_DOWNLOADER_H

#include <openssl/ssl.h>

#include "file_io.h"
#include "http_downloader.h"

class HttpsDownloader : public HttpDownloader {
  public:
  ~HttpsDownloader();

  HttpsDownloader(const struct DownloadSource& download_source);

  using HttpDownloader::HttpDownloader;

  private:
  bool receive_data(Connection& connection, Buffer& buffer) override;

  bool send_data(const Connection& connection, const Buffer& buffer) override;

  SSL* get_ssl(BIO* bio);

  bool init_connection(Connection& connection) override;

  std::unique_ptr<SocketOps> build_socket(const DownloadSource& download_source,
                                          bool proxy=false) override;
};

#endif
