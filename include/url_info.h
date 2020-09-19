#ifndef _URL_INFO_H
#define _URL_INFO_H

using namespace std;

enum class Protocol {
  HTTP,
  HTTPS,
  FTP
};
struct DownloadSource {
  DownloadSource(std::string ip = "", uint16_t port = 0) : ip(ip), port(port)
  {
  }

  std::string ip;
  std::string file_path_on_server;  // Including the file name
  std::string file_name;    // Without path
  std::string host_name;
  Protocol protocol;    // Enum protocol type
  uint16_t port;
};

class URLInfo
{
  public:
  ~URLInfo();
  URLInfo(const std::string& url);
  URLInfo(const std::string& ip, uint16_t port);
  struct DownloadSource get_download_source();

  /** Gets socket descriptor
   *
   * @return <connecting result, socket descriptor>
   */
  std::pair<bool, int>
    get_socket_descriptor(time_t timeout_interval=DEFAULT_TIMEOUT_SECONDS);

  private:
  constexpr static time_t DEFAULT_TIMEOUT_SECONDS = 5;
  std::string url;
  std::string ip;
  uint16_t port;
  struct DownloadSource download_source;
  int socket_descriptor;
};

#endif
