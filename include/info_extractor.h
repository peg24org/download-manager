#ifndef INFO_EXTRACTOR_H
#define INFO_EXTRACTOR_H

#include <memory>

#include "url_parser.h"
#include "socket_ops.h"
#include "transceiver.h"

class InfoExtractor
{
  public:
    InfoExtractor(const std::string& url, const std::string& http_proxy_url);

    Protocol get_protocol() const;
    std::string get_path() const;
    // 0: unknown file length.
    std::string get_file_name() const;
    size_t get_file_length() const;
    std::string get_host_name() const;
    uint16_t get_port() const;
    int get_one_socket_descriptor();
    std::unique_ptr<SocketOps> acquire_sock_ops();

  private:
    std::unique_ptr<SocketOps> get_socket_ops();
    std::pair<bool, std::string> check_link();
    std::string get_ip(const std::string& host_name) const;
    size_t get_file_length_ftp(Transceiver* transceiver, SocketOps* socket_ops);
    std::unique_ptr<SocketOps> connect_to_proxy(const std::string& dest_host,
                                                uint16_t dest_port);

    UrlParser url_parser;
    std::string ip;
    size_t file_length;
    std::unique_ptr<SocketOps> socket_ops;
    std::string http_proxy_url;
};

#endif
