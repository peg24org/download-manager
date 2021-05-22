#include "request_manager.h"

#include <sys/socket.h>

#include <iostream>

#include "socket_ops.h"

using namespace std;

RequestManager::RequestManager(unique_ptr<ConnectionManager> connection_manager)
  : connection_manager(move(connection_manager))
  , timeout_seconds(kDefaultTimeoutSeconds)
  , connections(1)
  , proxy_host("")
  , proxy_port(0)
  , keep_running(true)
{
}

void RequestManager::stop()
{
  keep_running.store(false);
}

bool RequestManager::resumable()
{
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
      this_thread::sleep_for(chrono::milliseconds(100));
      continue;
    }
    send_requests();
  }
}

pair<bool, string> RequestManager::check_redirected()
{
}

int RequestManager::set_descriptors()
{
  int max_fd = 0;
//  lock_guard<mutex> lock(request_mutex);
//  FD_ZERO(&writefds);
//  for (auto request: requests) {
//    int socket_desc = request.socket;
//    FD_SET(socket_desc, &writefds);
//    max_fd = (max_fd < socket_desc) ? socket_desc : max_fd;
//  }
//
  return max_fd;
}

bool RequestManager::send_requests()
{
  lock_guard<mutex> lock(request_mutex);
  for (Request& request : requests ) {
    if (!request.sent) {
      unique_ptr<SocketOps> sock_ops = connection_manager->acquire_sock_ops();
      const Buffer request_buf = generate_request_str(request);
      cerr << string(const_cast<Buffer&>(request_buf), request_buf.length()) << endl;
      int sock = sock_ops->get_socket_descriptor();
      request.sent = send_request(request_buf, sock);
      notify_dwl_available(request.request_index, move(sock_ops));
    }
  }
}

bool RequestManager::send_request(const Buffer& request, int socket)
{
  size_t sent_bytes = 0;
  size_t tmp_sent_bytes = 0;
  while (sent_bytes < request.length()) {
    tmp_sent_bytes = send(socket, const_cast<Buffer&>(request)+sent_bytes,
                          request.length(), 0);
    if (tmp_sent_bytes >= 0)
      sent_bytes += tmp_sent_bytes;
    else
      return false;
  }

  return true;
}

int RequestManager::connect()
{
}

bool RequestManager::request_available()
{
  lock_guard<mutex> lock(request_mutex);
  return !requests.empty();
}

Buffer RequestManager::generate_request_str(const Request& request)
{
  Buffer request_buffer;
  request_buffer << "GET " << connection_manager->get_path() << "/"
          << connection_manager->get_file_name() << " HTTP/1.1\r\nRange: bytes="
          << to_string(request.start_pos) << "-" << to_string(request.end_pos) << "\r\n"
          << "User-Agent: no_name_yet!\r\n"
          << "Accept: */*\r\n"
          << "Accept-Encoding: identity\r\n"
          << "Host:" << connection_manager->get_host_name() << ":"
          << connection_manager->get_port() << "\r\n\r\n";
  return request_buffer;
}

