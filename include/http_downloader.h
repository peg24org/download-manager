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
	char *buffer;
	void downloader_trd() override;	

	public:

	HttpDownloader(node_struct* node_data,
			const addr_struct addr_data,off_t pos,
			off_t trd_length, int index):Downloader(node_data,
				addr_data, pos, trd_length, index)
	{
	}
	void connect_to_server();
	off_t get_size() override;
	void call_node_status_changed(int recieved_bytes, int err_flag = 0);

};
#endif
