#ifndef URL_OPS_H
#define URL_OPS_H

#include <string>

enum class Protocol {
  HTTP,
  HTTPS,
  FTP
};

struct DownloadSource
{
  std::string ip;
  std::string file_path;  // Including the file name
  std::string file_name;    // Without path
  std::string host_name;
  Protocol protocol;    // Enum protocol type
  uint16_t port{0};

  std::string proxy_ip;
  uint16_t proxy_port{0};
};

// Url operations class.
class UrlOps
{
  public:
    /**
     *  The c-tor.
     *  @param url Url to be processed.
     */
    UrlOps(const std::string& url);

    // @return Host name of url.
    std::string get_hostname() const;

    // @return File path of url.
    std::string get_path() const;

    // @return File name of url.
    std::string get_file_name() const;

    // @return Ip of url.
    std::string get_ip() const;

    // @return Protocol of url.
    Protocol get_protocol() const;

    // @return Port of url.
    uint16_t get_port() const;

    void set_proxy(const std::string& proxy_url);
    uint16_t get_proxy_port() const;
    std::string get_proxy_ip() const;

  private:
    std::string get_url_hostname(const std::string& url) const;
    uint16_t get_url_port(const std::string& url) const;
    std::string get_host_ip(const std::string& hostname) const;

    std::string url;
    std::string host;
    uint16_t port;
    std::string proxy_host;
    uint16_t proxy_port;
};

#endif
