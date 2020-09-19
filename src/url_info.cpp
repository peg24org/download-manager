#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <regex>
#include <string>
#include <iostream>

#include "url_info.h"

using namespace std;

URLInfo::URLInfo(const string& url) : url(url), socket_descriptor(0)
{
}

URLInfo::URLInfo(const std::string& ip, uint16_t port)
  : download_source(DownloadSource(ip, port)), socket_descriptor(0)
{
}

URLInfo::~URLInfo()
{
}

struct DownloadSource URLInfo::get_download_source()
{
  smatch m;
  regex link_pattern("((http://|ftp://|https://)|())(.*?)(/|:(.+?)/)");

  if (regex_search(url, m, link_pattern)) {
    download_source.host_name = m[4];
    if (m[6].length() > 0)
      download_source.port = stoi(m[6]);
    download_source.file_path_on_server = '/' + m.suffix().str();
  }

  regex file_name_pattern("(.*?/.+/)(.*)");

  if (regex_search(url, m, file_name_pattern))
    download_source.file_name = m[2];

  regex http_pattern("http:|https:");
  regex ftp_pattern("ftp:");
  if (regex_search(url, m, http_pattern)) {
    if(m[0].str()=="http:") {
      download_source.protocol = Protocol::HTTP;
      if (download_source.port == 0)
        download_source.port = 80;
    }
    else if(m[0].str()=="https:") {
      download_source.protocol = Protocol::HTTPS;
      if (download_source.port == 0)
        download_source.port = 443;
    }
  }
  else if (regex_search(url, m, ftp_pattern)) {
    download_source.protocol = Protocol::FTP;
    if (download_source.port == 0) {
      download_source.port = 21;
    }
  }

  struct hostent *server;
  server = gethostbyname(download_source.host_name.c_str());
  if (!server){
    cerr << "Error, no such host." << endl;
    exit(0);
  }

  download_source.ip = string(inet_ntoa(*((struct in_addr*) server->h_addr)));

  regex encoding_character("%20");
  download_source.file_name =
    regex_replace(download_source.file_name, encoding_character, " ");

  return download_source;
}

pair<bool, int> URLInfo::get_socket_descriptor(time_t timeout_interval)
{
  if (download_source.port == 0 || download_source.ip == "")
    get_download_source();

  pair<bool, int> result = make_pair(false, 0);

  socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_descriptor < 0)
    return result;

  struct sockaddr_in dest_addr;
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(download_source.port);
  dest_addr.sin_addr.s_addr = inet_addr(download_source.ip.c_str());
  memset(&(dest_addr.sin_zero),'\0', sizeof(dest_addr.sin_zero));

  if (connect(socket_descriptor, (struct sockaddr *)&dest_addr,
              sizeof(struct sockaddr)) < 0) {
      return result;
  }
  else
    result.first = true;

  result.second = socket_descriptor;
  return result;
}
