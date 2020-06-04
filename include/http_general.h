#ifndef _HTTP_GENERAL_H
#define _HTTP_GENERAL_H

#include "downloader.h"
#include "definitions.h"
#include "node.h"

class HttpGeneral : public Downloader {
  public:
  HttpGeneral(FileIO& file_io, Logger& logger, node_struct* node_data,
      const struct addr_struct addr_data, size_t pos, size_t trd_length,
      int index)
    : Downloader(file_io, logger, node_data, addr_data, pos, trd_length, index)
    {}
  int check_link(string& redirected_url, size_t& file_size) override;

  protected:
  size_t get_size();
  // Different implementation for http and https, HttpsDownloader overrides
  //  this function
  virtual bool http_connect();
  string receive_header;
  bool check_redirection(string& redirect_url);

  private:
  constexpr static size_t MAX_HTTP_HEADER_LENGTH = 64 * 1024;

  void downloader_trd() override;
  char* get_header_delimiter_position(const char* buffer);
};

#endif
