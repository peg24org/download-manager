#ifndef _HTTPS_DOWNLOADER_H
#define _HTTPS_DOWNLOADER_H

#include "downloader.h"
#include "definitions.h"
#include <openssl/ssl.h>
#include "node.h"

class HttpsDownloader:public Downloader {

	public:

	HttpsDownloader(node_struct* node_data,	const addr_struct addr_data,
			off_t pos, off_t trd_length,int index) : Downloader(node_data,
				addr_data,	pos, trd_length, index)
				,port(0), sockfd(0), ssl(nullptr) {};

	~HttpsDownloader();
	void connect_to_server();
	off_t get_size() override;
	bool check_redirection(string& redirecting) override;

	bool regex_search_string(const string& input, const string& pattern,
			string& output);
	bool check_link(string& redirected_url, off_t& size);
	void call_node_status_changed(int recieved_bytes, int err_flag = 0);

	private:	

	void downloader_trd() override;
	void disconnect();
	bool check_ssl_error(int retval_from_function);
	bool ssl_send(const char* buffer, off_t size);
	bool ssl_recv(char* buffer, off_t size);

	uint16_t port;
	int	sockfd;
	SSL *ssl;

	// Receive buffer
	string receive_header;

	const static string HTTP_HEADER;// = "(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)";
};
#endif
