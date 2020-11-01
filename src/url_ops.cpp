#include "url_ops.h"

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <regex>
#include <string>
#include <iostream>
#include <exception>


using namespace std;

UrlOps::UrlOps(const string& url)
  : url(url)
  , proxy_host("")
  , proxy_port(0)
{
  UrlParameters parsed_url = parse_url(url);
  file_path = get<0>(parsed_url);
  file_name = get<1>(parsed_url);
  host_name = get<2>(parsed_url);
  port = get<3>(parsed_url);
  protocol = get<4>(parsed_url);
}

string UrlOps::get_hostname() const
{
  return host_name;
}

string UrlOps::get_path() const
{
  return file_path;
}

string UrlOps::get_file_name() const
{
  return file_name;
}

string UrlOps::get_ip() const
{
  return get_host_ip(host_name);
}

Protocol UrlOps::get_protocol() const
{
  return protocol;
}

uint16_t UrlOps::get_port() const
{
  return port;
}

string UrlOps::get_host_ip(const string& hostname) const
{
  struct hostent *server;
  server = gethostbyname(hostname.c_str());
  if (!server)
    throw runtime_error("cannot get ip.");

  return string(inet_ntoa(*((struct in_addr*) server->h_addr)));
}

void UrlOps::set_proxy(const string& proxy_url)
{
  UrlParameters parsed_url = parse_url(proxy_url + "/");
  proxy_host = get<2>(parsed_url);
  proxy_port = get<3>(parsed_url);
}

uint16_t UrlOps::get_proxy_port() const
{
  return proxy_port;
}

string UrlOps::get_proxy_ip() const
{
  return !proxy_host.empty() ? get_host_ip(proxy_host) : "";
}

UrlOps::UrlParameters UrlOps::parse_url(const std::string& url, bool proxy)
{
  Protocol protocol;
  size_t protocol_pos = url.find("://");
  string protocol_str;

  if (protocol_pos == string::npos)
    throw invalid_argument("invalid url protocol");
  else {
    protocol_str = url.substr(0, protocol_pos);
    auto it = kStandardProtocols.find(protocol_str);
    if (it != kStandardProtocols.end())
      protocol = it->second;
    else
      throw invalid_argument("unknown protocol");
  }

  string url_ = url.substr(protocol_pos+3, url_.length());
  size_t colon_pos = url_.find(":");
  const size_t first_slash = url_.find("/");
  size_t last_slash = url_.find_last_of("/");
  string port_str;
  uint16_t port;

  if (colon_pos != string::npos && first_slash != string::npos) {
    port_str = url_.substr(colon_pos+1, first_slash-colon_pos-1);
    port = stoi(port_str);
  }
  else {
    colon_pos = first_slash;
    auto it = kStandardPorts.find(protocol_str);
    if (it != kStandardPorts.end())
      port = it->second;
    else
      throw invalid_argument("unknown protocol");
  }

  if (proxy)
    return make_tuple("_", "_", host_name, port, protocol);
  string host_name = url_.substr(0, colon_pos);
  string file_path = url_.substr(first_slash, last_slash-first_slash+1);
  string file_name = url_.substr(last_slash+1);

  return make_tuple(file_path, file_name, host_name, port, protocol);
}

const std::map<const std::string, uint16_t> UrlOps::
  kStandardPorts =  {{"https", 443},
                     {"http", 80},
                     {"ftp", 21}};

const std::map<const std::string, Protocol> UrlOps::
  kStandardProtocols =  {{"https", Protocol::HTTPS},
                         {"http", Protocol::HTTP},
                         {"ftp", Protocol::FTP}};
