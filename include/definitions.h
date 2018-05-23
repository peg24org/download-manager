#ifndef _DEFINITIONS_H
#define _DEFINITIONS_H
#include <iostream>
#include <mutex>
typedef struct{
	int port;
	std::string ip;
	std::string file_path_on_server;
	std::string file_name_on_server;
	std::string host_name;
	bool encrypted;
	int protocol;		// Enum protocol type
}addr_struct;

enum protocol_type {
	kHttp,
	kFtp,
};

typedef struct {
	FILE*	fp;
	FILE*	log_fp;
	std::string log_buffer_str;
	std::mutex*	file_mutex;
	void* node;
	bool	resuming;
}node_struct;

#define LOG cout << "FILE"<<__FILE__<<" Line:"<<__LINE__<<std::endl;
#define HTTP_1_0
#define HTTP_1_1
#define FTP
#endif
