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
  requests.insert(pair(requests.size(), request));
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
    remove_sent_requests();
  }
}

bool RequestManager::request_available()
{
  lock_guard<mutex> lock(request_mutex);
  return !requests.empty();
}

void RequestManager::remove_sent_requests()
{
  vector<size_t> sent_requests;
  for (const auto& [index, request] : requests)
    if (request.sent)
      sent_requests.push_back(index);
  for (const size_t index : sent_requests)
    requests.erase(index);
}

