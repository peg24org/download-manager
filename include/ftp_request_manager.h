#ifndef _FTP_REQUEST_MANAGER_H
#define _FTP_REQUEST_MANAGER_H

#include <mutex>
#include <memory>
#include <atomic>
#include <vector>
#include <functional>

#include "request_manager.h"

class FtpRequestManager: public RequestManager
{
  public:
    using RequestManager::RequestManager;

  private:
    virtual void send_requests() override;
    // return <ip>:<port>
    std::string get_data_channel_addr(SocketOps* sock_ops);
    std::pair<std::string, uint16_t> get_ip_port_pair(const std::string& buffer);
    std::unique_ptr<SocketOps> open_data_channel(const std::string& ip,
                                                 uint16_t port);
    bool initialize(SocketOps* sock_ops, const std::string& dir);
    bool send_ftp_requst(const Request& request, SocketOps* sock_ops);
    std::pair<bool, std::string> send_ftp_command(const Buffer& command,
                                                  SocketOps* sock_ops);
};

#endif


