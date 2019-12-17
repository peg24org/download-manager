#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h> 

#include <regex>
#include <cstring>
#include <algorithm>

#include "ftp_downloader.h"

void FtpDownloader::connect_to_server()
{
	struct sockaddr_in dest_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(addr_data.port);
	dest_addr.sin_addr.s_addr = inet_addr(addr_data.ip.c_str());
	memset(&(dest_addr.sin_zero),'\0',8);
	if( connect(sockfd, (struct sockaddr *)&dest_addr,
				sizeof(struct sockaddr))< 0) {
		perror("ERROR connecting");
		exit(1);
	}
}

void FtpDownloader::disconnect()
{
	if (sockfd != 0){
		close(sockfd);
		sockfd = 0;
	}
}

bool FtpDownloader::check_link(string& redirected_url, size_t& size)
{
	connect_to_server();
	ftp_init();

	string reply;
	string stat, value;
	send_ftp_command("SIZE " + addr_data.file_name_on_server + "\r\n", stat,
			value);
	size = stoi(value);
	return false;
}

void FtpDownloader::ftp_init(string username, string password)
{
	string reply;
	send_ftp_command("", reply);
	string command_buffer = "USER anonymous\r\n";
	send_ftp_command("USER anonymous\r\n", reply);
	command_buffer = "PASS anonymous\r\n";
	send_ftp_command("PASS anonymous\r\n", reply);
	send_ftp_command("TYPE I\r\n", reply);
	send_ftp_command("PWD\r\n", reply);
	send_ftp_command("CWD " + addr_data.file_path_on_server.substr(0,
			addr_data.file_path_on_server.find(addr_data.file_name_on_server))
			+ "\r\n", reply);
}

void FtpDownloader::downloader_trd()
{
	connect_to_server();
	ftp_init();

	string stat;
	string reply;
	send_ftp_command("PASV\r\n", stat, reply);
	open_data_channel();
	send_ftp_command("LRECL " + to_string(trd_len) + "\r\n", stat, reply);
	send_ftp_command("REST " + to_string(pos) + "\r\n", stat, reply);
	send_ftp_command("RETR " + addr_data.file_name_on_server + "\r\n", stat,
      reply);
	char* buffer = new char[CHUNK_SIZE * sizeof(char)];
	size_t recieved_bytes = 0;
	while(recieved_bytes < trd_len){
		size_t bytes = 0;

		size_t scale = trd_len - recieved_bytes + 1;
		if (!ftp_data_receive(buffer, bytes,
          CHUNK_SIZE < scale ? CHUNK_SIZE : scale))
			exit(1);
		write_to_file(pos, bytes, buffer);//n=length
		pos += bytes;
		recieved_bytes += bytes;
		call_node_status_changed(bytes);
	}
	delete[] buffer;
}

bool FtpDownloader::send_ftp_command(const string& command, string& status,
		string& value)
{
	if (!socket_send(command.c_str(), command.length()))
		return false;

	string receive_header;
	receive_header.resize(1000);

	size_t received_bytes = 0;
	while (true){
		size_t number_of_bytes;
		if (!socket_receive(const_cast<char*>(receive_header.data()) +
					received_bytes,	number_of_bytes, 1000)){
			cerr << "Socket receive error" << endl;
		}
		received_bytes += number_of_bytes;

		/* RFC9559: A reply is defined to contain the 3-digit code, followed by
		 * Space */
		if (regex_search_string(receive_header, "(\\d{3}\\s)(.*)", status, 1)){
			// Reply of PASV, must get ip and port
			if (status == "227 "){
				string ip;
				regex_search_string(receive_header,
						"(\\d+,\\d+,\\d+,\\d+)", ip, 1);
				replace(ip.begin(), ip.end(), ',', '.');

				string port;
				regex_search_string(receive_header,
						"(\\d+,\\d+)(\\))", port, 1);
				string p1 = port.substr(0, port.find(','));
				string p2 = port.substr(port.find(p1) + p1.length() + 1,
						port.length() - p1.length());
				data_port = static_cast<uint8_t>(stoi(p1));
				data_port <<= 8;
				data_port |= static_cast<uint8_t>(stoi(p2));

				data_host = ip;
				value = ip + ":"+to_string(data_port);
			}
			else
				regex_search_string(receive_header, "(\\d{3}\\s)(\\d+)", value,
						2);
			return true;
		}
	}
	return false;
}

bool FtpDownloader::send_ftp_command(const string& command, string& status)
{
	string value;
	send_ftp_command(command, status, value);
	return false;
}

bool FtpDownloader::open_data_channel()
{
	struct sockaddr_in dest_addr;
	data_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(data_port);
	dest_addr.sin_addr.s_addr = inet_addr(data_host.c_str());
	memset(&(dest_addr.sin_zero),'\0',8);
	if( connect(data_sockfd, (struct sockaddr *)&dest_addr,
				sizeof(struct sockaddr))< 0) {
		perror("ERROR connecting");
		exit(1);
	}
	return true;
}


bool FtpDownloader::ftp_data_receive(char* buffer, size_t& received_len,
		size_t buffer_capacity)
{
	return (received_len =
      recv(data_sockfd, buffer, buffer_capacity, 0)) > 0 ? true : false;
}
