#include "connection_manager.h"

#include <netdb.h>
#include <arpa/inet.h>

#include <cstring>
#include <iostream>

#include "transceiver.h"
#include "pattern_finder.h"
#include "https_socket_ops.h"

using namespace std;

ConnectionManager::ConnectionManager(const string& url)
  : url_parser(UrlParser(url))
{
  while (true) {
    try {
      ip = get_ip(url_parser.get_host_name());
      if (!check_redirection().first)
        break;
    } catch (const runtime_error& e) {
      cerr << "Exception occurred, " << e.what() << endl;
    }
  }
}

Protocol ConnectionManager::get_protocol() const
{
  return url_parser.get_protocol();
}

string ConnectionManager::get_path() const
{
  return url_parser.get_path();
}

string ConnectionManager::get_file_name() const
{
  return url_parser.get_file_name();
}

unique_ptr<SocketOps> ConnectionManager::get_socket_ops()
{
  unique_ptr<SocketOps> socket_ops;
  Protocol protocol = url_parser.get_protocol();
  switch (protocol) {
    case Protocol::HTTP:
    case Protocol::FTP:
      socket_ops = make_unique<SocketOps>(ip, url_parser.get_port());
      break;
    case Protocol::HTTPS:
      socket_ops = make_unique<HttpsSocketOps>(ip, url_parser.get_port());
      break;
  }
  return socket_ops;
}

pair<bool, string> ConnectionManager::check_redirection()
{
  unique_ptr<Transceiver> transceiver;
  unique_ptr<SocketOps> socket_ops = get_socket_ops();
  socket_ops->connect();
  pair<bool, string> result = make_pair(false, "");

  switch (url_parser.get_protocol()) {
    case Protocol::FTP:
      return result;
      break;
    case Protocol::HTTP:
      transceiver = make_unique<Transceiver>();
      break;
    case Protocol::HTTPS:
      // FIXME Implement https transceiver.
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
    if (!transceiver->receive(recvd_header.seek(recvd_header.length()),
                              socket_ops.get())) {
      throw runtime_error(string("receiving failed, ") + __FUNCTION__);
    }
    if (pattern_finder.find_http_header_delimiter(recvd_header))
      break;
  }

  // Check header
  pair<bool, string> redirection;
  // Check redirection
  redirection = pattern_finder.find_redirection(recvd_header);
  if (redirection.first)
    url_parser = UrlParser(redirection.second);

  return redirection;
}

string ConnectionManager::get_ip(const string& host_name) const
{
  struct hostent *server;
  server = gethostbyname(host_name.c_str());
  if (!server)
    throw runtime_error("cannot get ip.");
  string ip(inet_ntoa(*((struct in_addr*) server->h_addr)));

  return ip;
}

