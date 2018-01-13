#include "http_downloader.h"
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring> //strlen()
#include <regex>
HttpDownloader::HttpDownloader(const addr_info dwl_str, off_t pos, off_t trd_length, void* node,int index)
{
	this->ip 	= dwl_str.ip;
	this->file_path	= dwl_str.file_path_on_server;
	this->port 	= dwl_str.port;
	this->file_name	= dwl_str.file_name_on_server;
	this->host_name	= dwl_str.host_name;
	this->index 	= index;
	this->pos	= pos;
	this->trd_len	= trd_length;
	this->node	= node;
}
void HttpDownloader::downloader_trd()
{	
	int n;
	buffer = (char *)malloc(500*sizeof(char));

	string command_buffer = "GET "+file_path+" HTTP/1.1\r\n"+
				"Range: bytes="+to_string(pos)+"-"+to_string(pos+trd_len)+"\r\n"
				"Host:"+host_name+":"+to_string(port)+"\r\n\r\n";
	if ( !sockfd )
		connect_to_server();
	n = send(sockfd, command_buffer.c_str(), command_buffer.length(),0);
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}
	n = 0 ;
	char* header_delimiter_pos = 0;
	int header_delimiter = 0;
	off_t temp_received_bytes = 0;
	while(recieved_bytes < trd_len){
		n = recv(sockfd, buffer, 500,0);
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
					if(stoi(m[2])/100!=2){
						cerr<<"Error: "<<m[2]<<" "<<m[3]<<endl;
						exit(1);
					}
					else
						cout<<m[2]<<" "<<m[3]<<endl;
				}
				else{
					cerr<<"Unknown error."<<endl;
					LOG;
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
void HttpDownloader::connect_to_server()
{
	struct sockaddr_in dest_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	memset(&(dest_addr.sin_zero),'\0',8);
	if( connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) < 0) {
		perror("ERROR connecting");
		exit(1);
	}
}
off_t HttpDownloader::get_size()
{	
	int n;
	string command_buffer = "HEAD "+file_path+" HTTP/1.1\r\n"+"Host:"+host_name+"\r\n\r\n";
	buffer = (char *)malloc(8192*sizeof(char));
	if ( !sockfd )
		connect_to_server();
	n = send(sockfd , command_buffer.c_str() , command_buffer.length() , 0);
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}
	n = 0 ;
	string length_str;
	while(true){
		n = recv(sockfd, buffer, 8192,0);
		if (n < 0) {
			perror("ERROR reading from socket");
			exit(1);
		}

		if(strstr(buffer,"\r\n\r\n")){
			string recv_buffer = string(buffer);
			smatch m;
			regex *e = new regex("(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)");
			bool found = regex_search(recv_buffer, m, *e);
			delete e;
			if(found && stoi(m[2])!=200){
				cerr<<"Error: "<<m[2]<<" "<<m[3]<<endl;
				exit(1);
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
		else
			buffer += n;
	}
	close(sockfd);
	free(buffer);
	return strtol(length_str.c_str(),NULL,10);

}
void HttpDownloader::call_node_status_changed(int recieved_bytes, int err_flag)
{
	void (*call_back_func_ptr)(void* node, int downloader_trd_index, off_t recieved_bytes, int stat_flag);
	call_back_func_ptr = Node::wrapper_to_get_status;
	call_back_func_ptr(node, index, recieved_bytes, 0);
}
