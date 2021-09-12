#include "http_request_manager.h"

using namespace std;

void HttpRequestManager::send_requests()
{
  lock_guard<mutex> lock(request_mutex);
  for (auto& [_, request] : requests ) {
    if (!request.sent) {
      unique_ptr<SocketOps> sock_ops = connection_manager->acquire_sock_ops();
      Buffer request_buf = generate_request_str(request);
      // TODO: check sent result
      request.sent = transceiver->send(request_buf, sock_ops.get());
      notify_dwl_available(request.request_index, move(sock_ops));
    }
  }
}

Buffer HttpRequestManager::generate_request_str(const Request& request)
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

