#ifndef _HTTP_DOWNLOADER_H
#define _HTTP_DOWNLOADER_H

#include "http_general.h"
#include "definitions.h"
#include "node.h"

class HttpDownloader : public HttpGeneral{

	public:
	using HttpGeneral::HttpGeneral;

	void connect_to_server() override;

	private:
	bool check_error(int len) const override;
	void disconnect() override;
	bool socket_send(const char* buffer, size_t len) override;
	bool socket_receive(char* buffer, size_t& received_len, size_t buffer_capacity) override;

};
#endif
