#ifndef _HTTP_PROXY_H
#define _HTTP_PROXY_H

#include <string>

class HttpProxy
{
  public:
    /**
     * C-tor.
     *
     * @param dest_host_name Download destination host name.
     * @param dest_port Download destination port.
     */
    HttpProxy(const std::string& dest_host_name, uint16_t dest_port);
    /**
     * Connect to proxy.
     *
     * @param socket_desc Socket descriptor of proxy to connect
     *
     */
    int connect(int socket_desc);

  private:
    const std::string kHttpRequest;
};

#endif
