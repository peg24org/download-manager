#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "definitions.h"
using namespace std;
class URLInfo
{
	std::string url;
	addr_struct dl_info;
	public:
	URLInfo(std::string url);
	addr_struct get_download_info();
};
