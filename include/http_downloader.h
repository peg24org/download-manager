#ifndef _HTTP_DOWNLOADER_H
#define _HTTP_DOWNLOADER_H

#include <vector>

#include "node.h"
#include "writer.h"
#include "downloader.h"

class HttpDownloader : public Downloader {

  public:
  HttpDownloader(const struct DownloadSource& download_source);

  using Downloader::Downloader;

  int check_link(std::string& redirected_url, size_t& file_size) override;

  protected:

  enum class HttpStatus {
    NONE = 0,
    OK = 200,
    PARTIAL_CONTENT = 206,
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    RANGE_NOT_SATICFIABLE = 416
  };

  std::string receive_header;

  bool send_requests() override;
  bool send_request(Connection& connection) override;
  size_t receive_from_connection(size_t index, char* buffer,
                                 size_t buffer_capacity) override;

  void receive_from_connection(size_t index, Buffer& buffer) override;

  size_t get_header_terminator_pos(const std::string& buffer) const;
  size_t get_size();
  bool check_redirection(std::string& redirect_url);

  // Parse the first line
  HttpStatus get_http_status(const char* buffer, size_t range);

  // Status of connections
  std::map<size_t, OperationStatus> connections_status;

  private:
  constexpr static size_t MAX_HTTP_HEADER_LENGTH = 64 * 1024;
  int set_descriptors() override;
};

#endif
