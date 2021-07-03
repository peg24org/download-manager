#ifndef _CONNECTIONION_MANAGER_HH
#define _CONNECTIONION_MANAGER_HH

#include <memory>

#include "url_parser.h"
#include "socket_ops.h"
#include "transceiver.h"

class ConnectionManager
{
  public:
    ConnectionManager(const std::string& url);

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

    UrlParser url_parser;
    std::string ip;
    size_t file_length;
    std::unique_ptr<SocketOps> socket_ops;
};

#endif
