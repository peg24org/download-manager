#ifndef _HTTP_GENERAL_H
#define _HTTP_GENERAL_H

#include "downloader.h"
#include "definitions.h"
#include "node.h"

class HttpGeneral : public Downloader {
	public:

	HttpGeneral(node_struct* node_data,	const addr_struct addr_data,size_t pos,
			size_t trd_length, int index)
		: Downloader(node_data, addr_data, pos, trd_length, index)
	{}

	bool check_link(string& redirected_url, size_t& size) override;

	private:
	
	constexpr static size_t MAX_HTTP_HEADER_LENGTH = 64 * 1024;

	void downloader_trd() override;	
	virtual bool check_error(int len) const = 0;
	virtual bool socket_send(const char* buffer, size_t len) = 0;
	virtual bool socket_receive(char* buffer, size_t& received_len,
			size_t buffer_capacity) = 0;


	struct hostent *server;

	protected:

	size_t get_size();
	string receive_header;
	bool check_redirection(string& redirect_url);
};

#endif
