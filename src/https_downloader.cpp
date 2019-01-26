#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/err.h>

#include "https_downloader.h"

void HttpsDownloader::connect_to_server()
{
	struct sockaddr_in dest_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(addr_data.port);
	dest_addr.sin_addr.s_addr = inet_addr(addr_data.ip.c_str());
	memset(&(dest_addr.sin_zero),'\0',8);
	
	if (connect(sockfd, (struct sockaddr *)&dest_addr,
				sizeof(struct sockaddr)) < 0) {
		perror("ERROR connecting");
		exit(1);
	}

	SSL_library_init();
	SSLeay_add_ssl_algorithms();
	SSL_load_error_strings();
	const SSL_METHOD *meth = TLSv1_2_client_method();
	SSL_CTX *ctx = SSL_CTX_new (meth);
	
	ssl = SSL_new(ctx);

	if (!ssl)
		cerr << "Error creating SSL." << endl;
	
	SSL_set_fd(ssl, sockfd);
	int err = SSL_connect(ssl);

	if (err <= 0){
		cerr << "Error creating SSL connection.  err=" << err << endl;
		exit(1);
	}
}

HttpsDownloader::~HttpsDownloader()
{
	disconnect();
}

void HttpsDownloader::disconnect()
{
	if (ssl != nullptr){
		SSL_free(ssl);
		ssl = nullptr;
	}
	if (sockfd != 0){
		close(sockfd);
		sockfd = 0;
	}
}

bool HttpsDownloader::socket_send(const char* buffer, size_t len)
{
	size_t sent_bytes = 0;
	size_t tmp_sent_bytes = 0;
	while (sent_bytes < len){
		if ((tmp_sent_bytes = SSL_write(ssl, buffer, len)) > 0)
			sent_bytes += tmp_sent_bytes;
		else {
			check_error(tmp_sent_bytes);
			cerr << " Https Err " << endl;
			return false;
		}
	}
	return true;
}

bool HttpsDownloader::socket_receive(char* buffer, size_t& received_len,
		size_t buffer_capacity)
{
	return (received_len = SSL_read(ssl, buffer, buffer_capacity)) > 0 ? true
		: false;
}

bool HttpsDownloader::check_error(int len) const
{
	if (len < 0) {
		int err = SSL_get_error(ssl, len);
		switch (err) {
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_WANT_READ:
				return true;
			case SSL_ERROR_ZERO_RETURN:
			case SSL_ERROR_SYSCALL:
			case SSL_ERROR_SSL:
			default:
				return false;
		}
	}
	return true;
}
