#include "info_extractor.h"

#include <netdb.h>
#include <arpa/inet.h>

#include <cstring>
#include <iostream>

#include "pattern_finder.h"
#include "ftp_transceiver.h"
#include "http_transceiver.h"
#include "https_socket_ops.h"
#include "https_transceiver.h"

using namespace std;

InfoExtractor::InfoExtractor(const string& url, const string& http_proxy_url)
  : url_parser(UrlParser(url))
  , http_proxy_url(http_proxy_url)
{
  while (true) {
    try {
      ip = get_ip(url_parser.get_host_name());
      if (!check_link().first)
        break;
    } catch (const runtime_error& e) {
      cerr << "Exception occurred, " << e.what() << endl;
    }
  }
}

Protocol InfoExtractor::get_protocol() const
{
  return url_parser.get_protocol();
}

string InfoExtractor::get_path() const
{
  return url_parser.get_path();
}

string InfoExtractor::get_file_name() const
{
  return url_parser.get_file_name();
}

size_t InfoExtractor::get_file_length() const
{
  return file_length;
}

string InfoExtractor::get_host_name() const
{
  return url_parser.get_host_name();
}

uint16_t InfoExtractor::get_port() const
{
  return url_parser.get_port();
}

int InfoExtractor::get_one_socket_descriptor()
{
  if (socket_ops->connect())
    return socket_ops->get_socket_descriptor();

  return -1;
}

unique_ptr<SocketOps> InfoExtractor::acquire_sock_ops()
{
  return get_socket_ops();
}

unique_ptr<SocketOps> InfoExtractor::get_socket_ops()
{
  unique_ptr<SocketOps> sock_ops;
  Protocol protocol = url_parser.get_protocol();
  switch (protocol) {
    case Protocol::HTTP:
    case Protocol::FTP:
      if (http_proxy_url.empty() == false)
        sock_ops = connect_to_proxy(ip, url_parser.get_port());
      else {
        sock_ops = make_unique<SocketOps>(ip, url_parser.get_port());
        sock_ops->connect();
      }
      break;
    case Protocol::HTTPS:
      sock_ops = make_unique<HttpsSocketOps>(ip, url_parser.get_port(),
                                             get_host_name());
      sock_ops->connect();
      break;
  }

  return sock_ops;
}

pair<bool, string> InfoExtractor::check_link()
{
  unique_ptr<Transceiver> transceiver;
  socket_ops = get_socket_ops();
  pair<bool, string> result = make_pair(false, "");

  switch (url_parser.get_protocol()) {
    case Protocol::FTP:
      transceiver = make_unique<FtpTransceiver>();
      file_length = get_file_length_ftp(transceiver.get(), socket_ops.get());
      return result;
      break;
    case Protocol::HTTP:
      transceiver = make_unique<HttpTransceiver>();
      break;
    case Protocol::HTTPS:
      transceiver = make_unique<HttpsTransceiver>();
      break;
  }

  // Use GET instead of HEAD, because some servers doesn't support HEAD
  // command.
  Buffer request(1024);
  request << "GET "
    << url_parser.get_path() << url_parser.get_file_name()
    << " HTTP/1.1\r\n"
    << "User-Agent: no_name_yet!\r\n"
    << "Accept: */*\r\n"
    << "Accept: */*\r\n"
    << "Accept-Encoding: identity\r\n"
    << "Host: " << url_parser.get_host_name() << ':' << url_parser.get_port()
    << "\r\n"
    << "Connection: Keep-Alive\r\n\r\n";

  constexpr static size_t MAX_HTTP_HEADER_LENGTH = 64 * 1024;
  Buffer header(MAX_HTTP_HEADER_LENGTH);
  if (!transceiver->send(request, socket_ops.get()))
    throw runtime_error(string("sending request failed, ") + __FUNCTION__);
  Buffer recvd_header;
  PatternFinder pattern_finder;
  while (true) {
    Buffer temp_recv_buffer;
    if (!transceiver->receive(temp_recv_buffer, socket_ops.get())) {
      throw runtime_error(string("receiving failed, ") + __FUNCTION__);
    }
    recvd_header << temp_recv_buffer;
    if (pattern_finder.find_http_header_delimiter(recvd_header) > 0)
      break;
  }
  // Check header
  pair<bool, string> redirection;
  // Check redirection
  redirection = pattern_finder.find_redirection(recvd_header);
  if (redirection.first)
    url_parser = UrlParser(redirection.second);
  else
    file_length = pattern_finder.find_file_length(recvd_header);

  return redirection;
}

string InfoExtractor::get_ip(const string& host_name) const
{
  struct hostent *server;
  server = gethostbyname(host_name.c_str());
  if (!server)
    throw runtime_error("cannot get ip.");
 // string ip(inet_ntoa(*((struct in_addr*) server->h_addr)));

  string ip(inet_ntoa(*((struct in_addr*)server->h_addr_list[0])));

  return ip;
}

size_t InfoExtractor::get_file_length_ftp(Transceiver* transceiver,
                                              SocketOps* socket_ops)
{
  FtpTransceiver* ftp_transceiver = dynamic_cast<FtpTransceiver*>(transceiver);
  ftp_transceiver->send_init_commands(socket_ops);

  Buffer size_command(string("SIZE ") + url_parser.get_path() +
                      url_parser.get_file_name()+ "\r\n");
  ftp_transceiver->send(size_command, socket_ops);

  Buffer recv_buffer;
  if (!ftp_transceiver->receive(recv_buffer, socket_ops))
      cerr << "Ftp receive error " << endl;

  string response(recv_buffer, recv_buffer.length());
  const string size_str = response.substr(response.find(' '),
                                          response.length());
  const size_t kFileSize = stoi(size_str);

  return kFileSize;
}

unique_ptr<SocketOps> InfoExtractor::connect_to_proxy(const string& dest_host,
                                                      uint16_t dest_port)
{
  UrlParser proxy_url_parser(http_proxy_url);
  const string proxy_ip = proxy_url_parser.get_host_name();
  const uint16_t proxy_port = proxy_url_parser.get_port();
  unique_ptr<SocketOps> proxy_sock_ops = make_unique<SocketOps>(proxy_ip,
                                                                proxy_port);
  const string proxy_connection_request =
    "CONNECT " + dest_host + ":" + to_string(dest_port) + " HTTP/1.1\r\n\r\n";
  HttpTransceiver proxy_transceiver;
  proxy_sock_ops->connect();
  Buffer proxy_req(proxy_connection_request);
  if (!proxy_transceiver.send(proxy_req, proxy_sock_ops.get()))
    throw runtime_error(string("sending request failed, ") + __FUNCTION__);
  Buffer recvd_header;
  PatternFinder pattern_finder;
  while (true) {
    Buffer temp_recv_buffer;
    if (!proxy_transceiver.receive(temp_recv_buffer, proxy_sock_ops.get()))
      throw runtime_error(string("receiving failed, ") + __FUNCTION__);
    recvd_header << temp_recv_buffer;
    if (pattern_finder.find_http_header_delimiter(recvd_header) > 0)
      break;
  }

  return proxy_sock_ops;
}
