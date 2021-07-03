#include "url_parser.h"

#include <regex>
#include <string>
#include <exception>

using namespace std;

UrlParser::UrlParser(const string& url) : url(url)
{
  UrlParameters parsed_url = parse_url();
  file_path = get<0>(parsed_url);
  file_name = get<1>(parsed_url);
  host_name = get<2>(parsed_url);
  port = get<3>(parsed_url);
  protocol = get<4>(parsed_url);
}

string UrlParser::get_host_name() const
{
  return host_name;
}

string UrlParser::get_path() const
{
  return file_path;
}

string UrlParser::get_file_name() const
{
  return file_name;
}

Protocol UrlParser::get_protocol() const
{
  return protocol;
}

uint16_t UrlParser::get_port() const
{
  return port;
}

UrlParser::UrlParameters UrlParser::parse_url()
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

  string url_ = url.substr(protocol_pos+3, url.length());
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

//  if (proxy)
//    return make_tuple("_", "_", host_name, port, protocol);
  string host_name = url_.substr(0, colon_pos);
  string file_path = url_.substr(first_slash, last_slash-first_slash+1);
  string file_name = url_.substr(last_slash+1);

  return make_tuple(file_path, file_name, host_name, port, protocol);
}

const std::map<const std::string, uint16_t> UrlParser::
  kStandardPorts =  {{"https", 443},
                     {"http", 80},
                     {"ftp", 21}};

const std::map<const std::string, Protocol> UrlParser::
  kStandardProtocols =  {{"https", Protocol::HTTPS},
                         {"http", Protocol::HTTP},
                         {"ftp", Protocol::FTP}};

