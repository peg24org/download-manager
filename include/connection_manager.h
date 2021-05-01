#ifndef _CONNECTIONION_MANAGER_HH
#define _CONNECTIONION_MANAGER_HH

#include <memory>

#include "url_parser.h"
#include "socket_ops.h"

class ConnectionManager
{
  public:
    ConnectionManager(const std::string& url);

    Protocol get_protocol() const;
    std::string get_path() const;
    std::string get_file_name() const;
    std::unique_ptr<SocketOps> get_socket_ops();

  private:
    std::pair<bool, std::string> check_redirection();
    std::string get_ip(const std::string& host_name) const;

    UrlParser url_parser;
    std::string ip;
};

#endif
