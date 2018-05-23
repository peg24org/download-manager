#include "https_downloader.h"
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <openssl/err.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <regex>
#include <cstdio>

const string HttpsDownloader::
	HTTP_HEADER = "(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)";

void HttpsDownloader::downloader_trd()
{	
	string command_buffer = "GET " + addr_data.file_path_on_server +
		" HTTP/1.1\r\nRange: bytes=" + to_string(pos) + "-" +
		to_string(pos+trd_len) + "\r\nHost:" + addr_data.host_name + ":" +
		to_string(addr_data.port) + "\r\n\r\n";

	if (sockfd == 0)
		connect_to_server();

	int n = SSL_write(ssl, command_buffer.c_str(), command_buffer.length()); 
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}
	n = 0 ;

	char* header_delimiter_pos = 0;
	int header_delimiter = 0;
	off_t temp_received_bytes = 0;
	char* buffer = new char[256*1024*sizeof(char)];

	while(recieved_bytes < trd_len){
		n = SSL_read(ssl, buffer, 256*1024);
		if (n < 0) {
			perror("ERROR reading from socket");
			exit(1);
		}
		if(!header_delimiter_pos){
			header_delimiter_pos = strstr(buffer,"\r\n\r\n");
			if(header_delimiter_pos){
				string recv_buffer = string(buffer);
				string value;
				bool found = regex_search_string(recv_buffer,
						"(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)", value);
				if(found){
					// 2xx
					if(stoi(value) / 100 != 2){
						cerr<<"Error occured "<<endl;
						exit(1);
					}
				}
				else{
					cerr<<"Unknown error."<<endl;
					exit(1);
				}

				// [\r\n\r\n]=4 Bytes
				header_delimiter = (int)(header_delimiter_pos - buffer) + 4; 
				temp_received_bytes = n - header_delimiter;

				//n=length
				write_to_file(pos, temp_received_bytes,
						buffer + header_delimiter);
				pos += temp_received_bytes;
				recieved_bytes += temp_received_bytes;
			}
			else {
				buffer += n;
				continue;
			}
		}
		else{
			write_to_file(pos, n, buffer);
			pos += n;
			recieved_bytes += n;
			temp_received_bytes = n;
		}
		call_node_status_changed(temp_received_bytes);
	}
	delete buffer;
}

void HttpsDownloader::connect_to_server()
{
	struct sockaddr_in dest_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(addr_data.port);
	dest_addr.sin_addr.s_addr = inet_addr(addr_data.ip.c_str());
	memset(&(dest_addr.sin_zero),'\0',8);
	if( connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) < 0) {
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

off_t HttpsDownloader::get_size()
{	
	string size_string;
	if (regex_search_string(receive_header, "(Content-Length: )(\\d+)", size_string))
		return  stoi(size_string);
	return 0;
}

void HttpsDownloader::call_node_status_changed(int recieved_bytes, int err_flag)
{
	void (*call_back_func_ptr)(void* node, int downloader_trd_index, off_t recieved_bytes, int stat_flag);
	call_back_func_ptr = Node::wrapper_to_get_status;
	call_back_func_ptr(node_data->node, index, recieved_bytes, 0);
}

bool HttpsDownloader::check_redirection(string& redirecting)
{
	if (regex_search_string(receive_header, HTTP_HEADER, redirecting))
	{
		// Check redirection
		if (regex_search_string(receive_header, "(Location: )(.+)",redirecting))
			return true;
		return false;
	}
}

bool HttpsDownloader::check_link(string& redirected_url, off_t& size)
{

	bool retval = false;
	string command_buffer = "HEAD "+addr_data.file_path_on_server+
		" HTTP/1.1\r\n"+"Host:"+addr_data.host_name+"\r\n\r\n";
	connect_to_server();

	receive_header.resize(MAX_HTTP_HEADER_LENGTH);

	int len = SSL_write(ssl, command_buffer.c_str(), command_buffer.length());
	check_ssl_error(len);
	len = 0;

	string recv_buffer;
	while (true){
		int number_of_bytes = SSL_read(ssl,
				const_cast<char*>(receive_header.data()),
				MAX_HTTP_HEADER_LENGTH);
		check_ssl_error(number_of_bytes);
		len += number_of_bytes;
		if (receive_header.find("\r\n\r\n") != string::npos)
			break;
	}
	if (check_redirection(redirected_url))
		return true;
	else {
		size = get_size();
		retval = false;
	}
	disconnect();
	return retval;
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
bool HttpsDownloader::check_ssl_error(int retval_from_function)
{
	if (retval_from_function < 0) {
		int err = SSL_get_error(ssl, retval_from_function);
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

bool HttpsDownloader::regex_search_string(const string& input, const string& pattern, string& output)
{
	smatch m;
	regex e(pattern);
	bool retval = regex_search(input, m, e);
	output = m[2];
	return retval;
}
