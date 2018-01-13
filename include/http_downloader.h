#ifndef _HTTP_DOWNLOADER_H
#define _HTTP_DOWNLOADER_H

#include "downloader.h"
#include "definitions.h"
#include "node.h"
class HttpDownloader:public Downloader {
	
	int port = 0,
	    sockfd = 0;
	off_t file_size = 0;
	struct hostent *server;
	string ip,
	       file_path,
	       file_name,
	       host_name;
	char *buffer;

	//int get_sockfd();//sockfd after connection
	void downloader_trd();	

	public:
	HttpDownloader(const addr_info, off_t pos, off_t trd_length, void *node,int index=0);//trd_length=-1: a constructor for getting info
	//void init_trd();
	void connect_to_server();
	off_t get_size();
	void call_node_status_changed(int recieved_bytes, int err_flag = 0);

};
#endif
