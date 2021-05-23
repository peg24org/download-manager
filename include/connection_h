#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <string>

class Connection {
  public:
    Connection(const std::string& host_name, uint16_t port=80);

    void set_proxy(const std::string& host_name, uint16_t port);
    bool connect();
    int get_socket_descriptor();

  private:
    bool socket_connect(const std::string& ip);
    std::string get_dest_ip(const std::string& dest_host_name) const;
    bool connect_to_proxy();

    std::string host_name;
    uint16_t port;
    std::string proxy_host_name;
    uint16_t proxy_port;

    int socket_descriptor;
};

#endif
