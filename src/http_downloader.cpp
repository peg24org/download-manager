#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h> 

#include <cstring>

#include "http_downloader.h"

void HttpDownloader::connect_to_server()
{
  struct sockaddr_in dest_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(addr_data.port);
  dest_addr.sin_addr.s_addr = inet_addr(addr_data.ip.c_str());
  memset(&(dest_addr.sin_zero),'\0',8);
  if( connect(sockfd, (struct sockaddr *)&dest_addr,
        sizeof(struct sockaddr))< 0) {
    perror("ERROR connecting");
    exit(1);
  }
}

bool HttpDownloader::check_error(int len) const
{
  if (len < 0) {
    perror("ERROR ");
    exit(1);
  }
  return true;
}

void HttpDownloader::disconnect()
{
  if (sockfd != 0){
    close(sockfd);
    sockfd = 0;
  }
}

bool HttpDownloader::socket_receive(char* buffer, size_t& received_len,
    size_t buffer_capacity)
{
  return (received_len = recv(sockfd, buffer, buffer_capacity, 0)) > 0 ? true
    : false;
}
