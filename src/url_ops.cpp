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

UrlOps::UrlOps(const string& url) : url(url)
{
}

string UrlOps::get_hostname() const
{
  smatch matched;
  regex link_pattern(R"X((//)((\w|\.)+))X");

  if (!regex_search(url, matched, link_pattern))
    throw invalid_argument("invalid url");

  return matched[2];
}

string UrlOps::get_path() const
{
  smatch matched;
  regex link_pattern(R"X(([a-z|0-9])(/[a-z|0-9].+))X");

  if (!regex_search(url, matched, link_pattern))
    throw invalid_argument("invalid url");

  string full_path = matched[2];
  size_t last_slash_pos = full_path.find_last_of('/');
  string path = full_path.substr(0, last_slash_pos + 1);

  return path;
}

string UrlOps::get_file_name() const
{
  smatch matched;
  regex link_pattern(R"X((:\/\/)(.+\/)(.+))X");

  if (!regex_search(url, matched, link_pattern))
    throw invalid_argument("invalid url");

  return matched[3];
}

string UrlOps::get_ip() const
{
  struct hostent *server;
  server = gethostbyname(get_hostname().c_str());
  if (!server)
    throw runtime_error("cannot get ip.");
  return string(inet_ntoa(*((struct in_addr*) server->h_addr)));
}

Protocol_ UrlOps::get_protocol() const
{
  smatch matched;
  regex link_pattern(R"X((\w+)(://))X");

  if (!regex_search(url, matched, link_pattern))
    throw invalid_argument("invalid url");
  Protocol_ protocol;

  string protocol_str = matched[1];
  if (protocol_str == "http")
    protocol = Protocol_::HTTP;
  else if (protocol_str == "https")
    protocol = Protocol_::HTTPS;
  else if (protocol_str == "ftp")
    protocol = Protocol_::FTP;
  else
    throw invalid_argument("invalid protocol");

  return protocol;
}

uint16_t UrlOps::get_port() const
{
  uint16_t port;
  smatch matched;
  regex link_pattern(R"X((:)(\d+))X");

  if (regex_search(url, matched, link_pattern))
    port = stoi(matched[2]);
  else {
    Protocol_ protocol = get_protocol();
    switch (protocol) {
      case Protocol_::HTTP:
        port = 80;
        break;
      case Protocol_::HTTPS:
        port = 443;
        break;
      case Protocol_::FTP:
        port = 21;
        break;
      default:
        throw invalid_argument("invalid url, port cannot be retrieved.");
    }
  }

  return port;
}
