#ifndef _HTTPS_DOWNLOADER_H
#define _HTTPS_DOWNLOADER_H

#include <openssl/ssl.h>

#include "definitions.h"
#include "http_general.h"

class HttpsDownloader:public HttpGeneral {
	public:
	~HttpsDownloader();
	HttpsDownloader(node_struct* node_data,	const struct addr_struct addr_data,
			size_t pos, size_t trd_length,int index)
		: HttpGeneral(node_data, addr_data, pos, trd_length, index)
		, ssl(nullptr){};
	void connect_to_server() override;

	private:	
	void disconnect() override;
	bool socket_send(const char* buffer, size_t len) override;
	bool socket_receive(char* buffer, size_t& len, size_t buffer_capacity) override;
	bool check_error(int len) const override;
	SSL *ssl;
};

#endif
