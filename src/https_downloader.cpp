#include "https_downloader.h"
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <openssl/err.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring> //strlen()
#include <regex>
void HttpsDownloader::downloader_trd()
{	
	int n;
	buffer = (char *)malloc(256*1024*sizeof(char));

	string command_buffer = "GET "+addr_data.file_path_on_server+" HTTP/1.1\r\n"+
				"Range: bytes="+to_string(pos)+"-"+to_string(pos+trd_len)+"\r\n"
				"Host:"+addr_data.host_name+":"+to_string(addr_data.port)+"\r\n\r\n";
	if ( !sockfd )
		connect_to_server();
	n = SSL_write(ssl, command_buffer.c_str(), command_buffer.length()); 
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}
	n = 0 ;
	char* header_delimiter_pos = 0;
	int header_delimiter = 0;
	off_t temp_received_bytes = 0;
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
				smatch m;
				regex *e = new regex("(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)");
				bool found = regex_search(recv_buffer, m, *e);
				delete e;
				if(found){
					if(stoi(m[2].str())/100!=2){
						cerr<<"Error: "<<m[2]<<" "<<m[3]<<endl;
						exit(1);
					}
				}
				else{
					cerr<<"Unknown error."<<endl;
					exit(1);
				}
				header_delimiter = (int)(header_delimiter_pos-buffer)+4;// [\r\n\r\n]=4 Bytes
				temp_received_bytes = n-header_delimiter;
				write_to_file(pos, temp_received_bytes, buffer+header_delimiter);//n=length
				pos += temp_received_bytes;
				recieved_bytes += temp_received_bytes;
			}
			else {
				buffer += n;
				continue;
			}
		}
		else{
			write_to_file(pos, n, buffer);//n=length
			pos += n;
			recieved_bytes += n;
			temp_received_bytes = n;
		}
		call_node_status_changed(temp_received_bytes);
	}
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
	
	int sock;

	ssl = SSL_new(ctx);
	if (!ssl)
		cerr << "Error creating SSL." << endl;
	sock = SSL_get_fd(ssl);
	SSL_set_fd(ssl, sockfd);
	int err = SSL_connect(ssl);
	if (err <= 0) {
		cerr << "Error creating SSL connection.  err=" << err << endl;
	}

}
off_t HttpsDownloader::get_size()
{	
	int n;
	string command_buffer = "HEAD "+addr_data.file_path_on_server+" HTTP/1.1\r\n"+"Host:"+addr_data.host_name+"\r\n\r\n";
	buffer = (char *)malloc(8192*sizeof(char));
	if ( !sockfd )
		connect_to_server();
	int len = SSL_write(ssl, command_buffer.c_str(), command_buffer.length());
	if (len < 0) {
		int err = SSL_get_error(ssl, len);
		switch (err) {
			case SSL_ERROR_WANT_WRITE:
				return 0;
			case SSL_ERROR_WANT_READ:
				return 0;
			case SSL_ERROR_ZERO_RETURN:
			case SSL_ERROR_SYSCALL:
			case SSL_ERROR_SSL:
			default:
				return -1;
		}
	}
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}
	n = 0 ;
	string length_str;
	string recv_buffer;
	while(true){
        	n += SSL_read(ssl, buffer+n , 8192);
		if (n < 0) {
			perror("ERROR reading from socket");
			exit(1);
		}

		if(strstr(buffer,"\r\n\r\n")){
			recv_buffer = string(buffer);
			smatch m;
			regex *e = new regex("(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)");
			bool found = regex_search(recv_buffer, m, *e);
			delete e;
			if(found){
				if(stoi(m[2]) != 200){
					cerr<<"Error: "<<m[2]<<" "<<m[3]<<endl;
					exit(1);
				}
			}

			e = new regex("(Content-Length: )(\\d+)");
			found = regex_search(recv_buffer, m, *e);
			delete e;
			if(found){
				length_str = m[2];
				cout<<"Content-Length: "<<length_str<<endl;
			}
			break;
		}
	}
	close(sockfd);
	free(buffer);
	buffer = nullptr;
	return strtol(length_str.c_str(),NULL,10);

}
void HttpsDownloader::call_node_status_changed(int recieved_bytes, int err_flag)
{
	void (*call_back_func_ptr)(void* node, int downloader_trd_index, off_t recieved_bytes, int stat_flag);
	call_back_func_ptr = Node::wrapper_to_get_status;
	call_back_func_ptr(node_data->node, index, recieved_bytes, 0);
}
