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
#include "info_extractor.h"

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
  Request() : socket(0), end_pos(0), start_pos(0), request_index(0)
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
    RequestManager(std::unique_ptr<InfoExtractor> info_extractor,
                   std::unique_ptr<Transceiver> transceiver);
    void stop();
    void set_proxy(std::string& host, uint32_t port);
    //void add_request(size_t start_pos, size_t length, uint16_t request_index);
    void add_request(size_t start_pos, size_t end_pos, uint16_t request_index);

    void register_dwl_notify_cb(DwlAvailNotifyCB dwl_notify_cb);

  protected:
    std::unique_ptr<InfoExtractor> info_extractor;
    std::unique_ptr<Transceiver> transceiver;

    std::string proxy_host;
    uint32_t proxy_port;

    std::mutex request_mutex;
    std::map<size_t, Request> requests;

    DwlAvailNotifyCB notify_dwl_available;

  private:
    void run() override;
    virtual void send_requests() = 0;
    void remove_sent_requests();
    bool request_available();
    std::atomic<bool> keep_running;
};

#endif

