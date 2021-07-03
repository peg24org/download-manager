#include "request_manager.h"

#include <iostream>

#include "socket_ops.h"

using namespace std;

RequestManager::RequestManager(unique_ptr<ConnectionManager> connection_manager,
                               unique_ptr<Transceiver> transceiver)
  : connection_manager(move(connection_manager))
  , transceiver(move(transceiver))
  , proxy_host("")
  , proxy_port(0)
  , keep_running(true)
{
}

void RequestManager::stop()
{
  keep_running.store(false);
}

void RequestManager::set_proxy(string& host, uint32_t port)
{
  proxy_host = host;
  proxy_port = port;
}

void RequestManager::add_request(size_t start_pos, size_t end_pos,
                                 uint16_t request_index)
{
  lock_guard<mutex> lock(request_mutex);
  const Request request{0, end_pos, start_pos, request_index};
  requests.push_back(request);
}

void RequestManager::register_dwl_notify_cb(DwlAvailNotifyCB dwl_notify_cb)
{
  notify_dwl_available = dwl_notify_cb;
}

void RequestManager::run()
{
  while (keep_running) {
    if (!request_available()) {
      // TODO implement conditional variable.
      this_thread::sleep_for(chrono::milliseconds(100));
      continue;
    }
    send_requests();
  }
}

bool RequestManager::request_available()
{
  lock_guard<mutex> lock(request_mutex);
  return !requests.empty();
}

