#ifndef _HTTP_DOWNLOADER_H
#define _HTTP_DOWNLOADER_H

#include <vector>

#include "node.h"
#include "writer.h"
#include "downloader.h"

class HttpDownloader : public Downloader {
  public:
  HttpDownloader(const struct DownloadSource& download_source,
                 const std::vector<int>& socket_descriptors);

  HttpDownloader(const struct DownloadSource& download_source,
                 std::vector<int>& socket_descriptors,
                 std::unique_ptr<Writer> writer,
                 ChunksCollection& chunks_collection,
                 long int timeout);

  int check_link(std::string& redirected_url, size_t& file_size) override;

  protected:
  std::string receive_header;

  virtual void send_request(size_t index);
  size_t get_header_delimiter_position(const char* buffer);
  size_t get_size();
  bool check_redirection(std::string& redirect_url);

  // Status of connections
  std::map<size_t, OperationStatus> connections_status;

  private:
  constexpr static size_t MAX_HTTP_HEADER_LENGTH = 64 * 1024;

  void downloader_trd() override;
};

#endif
