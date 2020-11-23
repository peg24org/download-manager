#ifndef _FTP_DOWNLOADER_H
#define _FTP_DOWNLOADER_H

#include <vector>

#include "node.h"
#include "buffer.h"
#include "writer.h"
#include "downloader.h"

class FtpDownloader : public Downloader {
  public:
  FtpDownloader(const struct DownloadSource& download_source);

  FtpDownloader(const struct DownloadSource& download_source,
                std::unique_ptr<Writer> writer,
                ChunksCollection& chunks_collection,
                time_t timeout_seconds,
                int number_of_connections=1);

  int check_link(std::string& redirected_url, size_t& file_size) override;

  protected:
  std::string receive_header;

  size_t get_header_delimiter_position(const char* buffer);
  size_t get_size();
  bool check_redirection(std::string& redirect_url);

  // Status of connections
  std::map<size_t, OperationStatus> connections_status;

  private:
  constexpr static size_t MAX_HTTP_HEADER_LENGTH = 64 * 1024;

  void ftp_init(Connection& connection, std::string username="anonymous",
                std::string password="anonymous");

  void ftp_init(std::string username="anonymous",
                std::string password="anonymous");
  bool send_ftp_command(Connection& connection, const std::string& command,
                        std::string& result);

  virtual bool receive_data(Connection& connection, char* buffer,
                            size_t& recv_len, size_t buffer_capacity) override;

  bool send_request(Connection& connection) override;

  bool send_requests() override;
  int set_descriptors() override;
  size_t receive_from_connection(size_t index, char* buffer,
                                 size_t buffer_capacity) override;

  void receive_from_connection(size_t index, Buffer& buffer) override;

  std::vector<std::string> split_string(const std::string& buffer,
                                        char delimiter);
  std::pair<std::string, uint16_t> get_data_ip_port(const std::string& buffer);
  void open_data_channel(Connection& connection, const std::string& ip,
                         uint16_t port);
  bool ftp_receive_data(Connection& connection, char* buffer,
                        size_t& received_len, size_t buffer_capacity);

  bool ftp_receive_data(Connection& connection, Buffer& buffer);
};

#endif

