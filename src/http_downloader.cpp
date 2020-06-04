#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h> 

#include <cstring>
#include <iostream>

#include "http_downloader.h"

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
