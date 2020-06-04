#ifndef _FTP_DOWNLOADER_H
#define _FTP_DOWNLOADER_H

#include "node.h"
#include "logger.h"
#include "file_io.h"
#include "definitions.h"

class FtpDownloader : public Downloader{
  public:
  FtpDownloader(FileIO& file_io, Logger& logger ,node_struct* node_data,
      const struct  addr_struct addr_data, size_t pos, size_t trd_length,
      int index)
    : Downloader(file_io, logger, node_data, addr_data, pos, trd_length, index)
    , data_port(0)
    , data_sockfd(0){}
  void disconnect() override;
  int check_link(string& redirect_url, size_t& size) override;

  private:
  void downloader_trd() override;
  bool send_ftp_command(const string& command, string& reply);
  bool send_ftp_command(const string& command, string& status, string& value);
  bool open_data_channel();

  //bool socket_send(const char* buffer, size_t len);
  bool ftp_data_receive(char* buffer, size_t& received_len,
      size_t buffer_capacity);

  // TODO: add error checking
  void ftp_init(string username = "anonymous",
      string password = "anonymous");
  map<int, string> ftp_response = {
    {200, "Command okay."},
    {220, "Service ready for new user."},
    {230, "User logged in, proceed."},
    {331, "User name okay, need password."}
  };

  string data_host;
  int data_port;
  int data_sockfd;
};
#endif
