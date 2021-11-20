#include "http_request_manager.h"

#include <sstream>

#include "pattern_finder.h"

using namespace std;

void HttpRequestManager::send_requests()
{
  Request request;
  {
    if (requests.size() == 0)
      return;
    lock_guard<mutex> lock(request_mutex);
    request = requests.begin()->second;;
    requests.erase(requests.begin());
  }

  unique_ptr<SocketOps> sock_ops = info_extractor->acquire_sock_ops();
  Buffer request_buf = generate_request_str(request);
  bool response_ok = false;
  if (transceiver->send(request_buf, sock_ops.get())) {
    Buffer header_buffer;
    while(true) {
      Buffer temp_buffer(1);
      if (transceiver->receive(temp_buffer, sock_ops.get())) {
        header_buffer << temp_buffer;
        PatternFinder pattern_finder;
        if (pattern_finder.find_http_header_delimiter(header_buffer) > 0) {
          stringstream header_stream;
          header_stream.write(header_buffer, header_buffer.length());
          uint16_t response;
          string _;
          header_stream >> _ >> response >> _;
          if (response == 206 || response == 200)
            response_ok = true;
          break;
        }
      }
    }
  }
  if (response_ok)
    notify_dwl_available(request.request_index, move(sock_ops));
}

Buffer HttpRequestManager::generate_request_str(const Request& request)
{
  Buffer request_buffer;
  request_buffer << "GET " << info_extractor->get_path() << "/"
          << info_extractor->get_file_name() << " HTTP/1.1\r\nRange: bytes="
          << to_string(request.start_pos) << "-"
          << to_string(request.end_pos) << "\r\n"
          << "User-Agent: no_name_yet!\r\n"
          << "Accept: */*\r\n"
          << "Accept-Encoding: identity\r\n"
          << "Host:" << info_extractor->get_host_name() << ":"
          << info_extractor->get_port() << "\r\n\r\n";
  return request_buffer;
}

