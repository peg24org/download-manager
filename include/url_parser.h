#ifndef _URL_PARSER_H
#define _URL_PARSER_H

#include <map>
#include <string>

enum class Protocol {
  HTTP,
  HTTPS,
  FTP
};


class UrlParser
{
  public:
    /**
     *  The c-tor.
     *
     *  @param url Url to be processed.
     *  @throws invalid_argument in case of using invalid protocol.
     *  @throws invalid_argument in case of using unknown protocol.
     */
    UrlParser(const std::string& url);

    // @return Host name of url.
    std::string get_host_name() const;

    // @return Full file path of url.
    std::string get_path() const;

    // @return File name of url.
    std::string get_file_name() const;

    // @return Protocol of url.
    Protocol get_protocol() const;

    // @return Port of url.
    uint16_t get_port() const;

  private:
    // <path> <file_name> <hostname> <port> <protocol>
    using UrlParameters = std::tuple<std::string, std::string, std::string,
                                     uint16_t, Protocol>;
    static const std::map<const std::string, uint16_t> kStandardPorts;
    static const std::map<const std::string, Protocol> kStandardProtocols;

    UrlParameters parse_url();

    std::string url;
    std::string host_name;
    std::string file_path;
    std::string file_name;
    uint16_t port;
    Protocol protocol;
};

#endif

