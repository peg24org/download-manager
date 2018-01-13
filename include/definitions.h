#ifndef _DEFINITIONS_H
#define _DEFINITIONS_H
#include <iostream>
typedef struct download_str{
	int port;
	std::string ip;
	std::string file_path_on_server;
	std::string file_name_on_server;
	std::string host_name;
}addr_info;
enum protocol_type {
	http_1_0,
	http_1_1,
	ftp,
};
#define LOG cout << "FILE"<<__FILE__<<" Line:"<<__LINE__<<std::endl;
#define HTTP_1_0
#define HTTP_1_1
#define FTP
#endif
