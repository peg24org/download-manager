#include <regex>
#include <cstdlib>
#include <cassert>

#include "node.h"
#include "definitions.h"
#include "http_general.h"

void HttpGeneral::downloader_trd()
{
	string command_buffer = "GET " + addr_data.file_path_on_server +
		" HTTP/1.1\r\nRange: bytes=" + to_string(pos) + "-" +
		to_string(pos + trd_len) + "\r\nHost:" + addr_data.host_name + ":" +
		to_string(addr_data.port) + "\r\n\r\n";

	if ( !sockfd )
		connect_to_server();

	if (!socket_send(command_buffer.c_str(), command_buffer.length()))
		exit(1);

	char* buffer = new char[CHUNK_SIZE * sizeof(char)];
	char* header_delimiter_pos = 0;
	int header_delimiter = 0;
	size_t temp_received_bytes = 0;
	size_t recieved_bytes = 0;
	while(recieved_bytes < trd_len){
		size_t bytes = 0;
		if (!socket_receive(buffer, bytes, CHUNK_SIZE))
			exit(1);
		if(header_delimiter_pos  == 0) {
			header_delimiter_pos = strstr(buffer,"\r\n\r\n");
			if(header_delimiter_pos) {
				string recv_buffer = string(buffer);
				smatch m;
				regex e("(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)");
				bool found = regex_search(recv_buffer, m, e);
				if(found) {
					if(stoi(m[2].str())/100!=2){
						cerr<< __LINE__ << " Error: "<<m[2]<<" "<<m[3]<<endl;
						exit(1);
					}
				}
				else{
					cerr<<"Unknown error."<<endl;
					exit(1);
				}

				// [\r\n\r\n]=4 Bytes
				header_delimiter = static_cast<int>((header_delimiter_pos -
							buffer) + 4);
				temp_received_bytes = bytes - header_delimiter;
				write_to_file(pos, temp_received_bytes, buffer
						+ header_delimiter);//n=length
				pos += temp_received_bytes;
				recieved_bytes += temp_received_bytes;
			}
			else {
				buffer += bytes;
				continue;
			}
		}
		else{
			write_to_file(pos, bytes, buffer);//n=length
			pos += bytes;
			recieved_bytes += bytes;
			temp_received_bytes = bytes;
		}
		call_node_status_changed(temp_received_bytes);
	}
	delete[] buffer;
}

size_t HttpGeneral::get_size()
{	
	string size_string;
	if (regex_search_string(receive_header, "(Content-Length: )(\\d+)",
				size_string))
		return  strtoul(static_cast<const char*>(size_string.c_str()), nullptr, 0);
	return 0;
}

bool HttpGeneral::check_redirection(string& redirecting)
{
	if (regex_search_string(receive_header, HTTP_HEADER, redirecting))
	{
		// Check redirection
		if (regex_search_string(receive_header, "(Location: )(.+)",redirecting))
			return true;
		return false;
	}
	return false;
}

bool HttpGeneral::check_link(string& redirected_url, size_t& size)
{
	bool retval = false;
	string command_buffer = "HEAD " + addr_data.file_path_on_server +
		" HTTP/1.1\r\nHost:" + addr_data.host_name + "\r\n\r\n";
	connect_to_server();

	receive_header.resize(MAX_HTTP_HEADER_LENGTH);

	if (!socket_send(command_buffer.c_str(), command_buffer.length()))
		exit(1);
	size_t len = 0;
	char* buffer = new char[CHUNK_SIZE * sizeof(char)];
	while (true) {
		size_t number_of_bytes;
		if (!socket_receive(const_cast<char*>(receive_header.data()),
					number_of_bytes, MAX_HTTP_HEADER_LENGTH))
			exit(1);
		len += number_of_bytes;
		if (receive_header.find("\r\n\r\n") != string::npos)
			break;
	}

	if (check_redirection(redirected_url)) {
		return true;
	}
	else {
		size = get_size();
		retval = false;
	}

	disconnect();
	delete[] buffer;
	return retval;
}
