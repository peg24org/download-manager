#ifndef _REQUEST_MANAGER_H
#define _REQUEST_MANAGER_H

#include <mutex>
#include <memory>
#include <atomic>
#include <vector>
#include <functional>

#include "thread.h"
#include "buffer.h"
#include "socket_ops.h"
#include "transceiver.h"
#include "connection_manager.h"

constexpr time_t kDefaultTimeoutSeconds = 5;

// index, socket
using DwlAvailNotifyCB =
  std::function<void(uint16_t, std::unique_ptr<SocketOps> sock_ops)>;

struct Request
{
  Request(int socket, size_t end_pos, size_t start_pos, uint16_t request_index)
    : socket(socket)
    , end_pos(end_pos)
    , start_pos(start_pos)
    , request_index(request_index)
    , sent(false)
  {
  }
  int socket;
  size_t end_pos;
  size_t start_pos;
  uint16_t request_index;
  bool sent;
};

class RequestManager : public Thread
{
  public: RequestManager(std::string url);
    RequestManager(std::unique_ptr<ConnectionManager> connection_manager);
    void stop();
    bool resumable();
    size_t get_size();
    void set_timeout(int timeout);
    void set_connections(int connections);
    void set_proxy(std::string& host, uint32_t port);
    void add_request(size_t start_pos, size_t length,
                     uint16_t request_index);

    void register_dwl_notify_cb(DwlAvailNotifyCB dwl_notify_cb);

  protected:
    std::unique_ptr<ConnectionManager> connection_manager;
    time_t timeout_seconds;
    int connections;

    std::string proxy_host;
    uint32_t proxy_port;

    fd_set writefds;
    std::mutex request_mutex;
    std::vector<Request> requests;

    DwlAvailNotifyCB notify_dwl_available;

  private:
    void run() override;
    std::pair<bool, std::string> check_redirected();
    bool send_requests();
    virtual bool send_request(const Buffer& request, SocketOps* sock_ops);
    // Connect to address and return socket.
    int connect();
    bool request_available();
    Buffer generate_request_str(const Request& request);
    std::atomic<bool> keep_running;
    Transceiver transceiver;
};

#endif

