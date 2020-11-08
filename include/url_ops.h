#ifndef _URL_OPS_H
#define _URL_OPS_H

#include <map>
#include <string>

enum class Protocol {
  HTTP,
  HTTPS,
  FTP
};

struct DownloadSource
{
  std::string ip;
  std::string file_path;    // Excluding the file name
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
     *
     *  @param url Url to be processed.
     *  @throws invalid_argument in case of using invalid protocol.
     *  @throws invalid_argument in case of using unknown protocol.
     */
    UrlOps(const std::string& url);

    // @return Host name of url.
    std::string get_hostname() const;

    // @return File path of url.
    std::string get_path() const;

    // @return File name of url.
    std::string get_file_name() const;

    /**
     * @return Ip of url.
     * @throws runtime_error if cannot get ip.
     */
    std::string get_ip() const;

    // @return Protocol of url.
    Protocol get_protocol() const;

    // @return Port of url.
    uint16_t get_port() const;

    /**
     * Set the proxy address.
     *
     * @param proxy_url Proxy address, for example: http://127.0.0.1:8080
     * @throws invalid_argument exception in case of using invalid url.
     */
    void set_proxy(const std::string& proxy_url);

    // @return Port of proxy.
    uint16_t get_proxy_port() const;

    // @return Ip of proxy.
    std::string get_proxy_ip() const;

  private:
    // <path> <file_name> <hostname> <port> <protocol>
    using UrlParameters = std::tuple<std::string, std::string, std::string,
                                     uint16_t, Protocol>;
    static const std::map<const std::string, uint16_t> kStandardPorts;
    static const std::map<const std::string, Protocol> kStandardProtocols;

    UrlParameters parse_url(const std::string& url, bool proxy=false);

    std::string get_host_ip(const std::string& hostname) const;

    std::string url;
    std::string host_name;
    std::string file_path;
    std::string file_name;
    uint16_t port;
    Protocol protocol;
    std::string proxy_host;
    uint16_t proxy_port;
};

#endif
