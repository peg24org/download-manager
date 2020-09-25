#include "ftp_downloader.h"

#include <regex>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <iostream>

#include <arpa/inet.h>

#include "node.h"

FtpDownloader::FtpDownloader(const struct DownloadSource& download_source,
                               const std::vector<int>& socket_descriptors)
  : Downloader(download_source, socket_descriptors)
{
  for (size_t index = 0; index < socket_descriptors.size(); ++index)
    connections[index].sock_desc = socket_descriptors[index];
}

FtpDownloader::FtpDownloader(const struct DownloadSource& download_source,
                               std::vector<int>& socket_descriptors,
                               std::unique_ptr<Writer> writer,
                               ChunksCollection& chunks_collection)
  : Downloader(download_source, socket_descriptors, move(writer),
               chunks_collection)
{
  for (auto chunk : chunks_collection) {
    connections[chunk.first].chunk = chunk.second;
    connections[chunk.first].sock_desc = socket_descriptors.at(chunk.first);
    connections[chunk.first].status = OperationStatus::NOT_STARTED;
  }
}

int FtpDownloader::check_link(string& redirected_url, size_t& size)
{
  ftp_init();

  string reply;
  const string command = "SIZE " + download_source.file_name + "\r\n";
  send_ftp_command(connections[0], command, reply);

  stringstream reply_stream(reply);
  size_t status;
  reply_stream >> status >> size;

  return 0;
}

void FtpDownloader::ftp_init(string username, string password)
{
  string reply;
  for (size_t index = 0; index < connections.size(); ++index) {
    Connection& connection = connections[index];

    send_ftp_command(connection, "", reply);
    send_ftp_command(connection, "USER " + username + "\r\n", reply);
    send_ftp_command(connection, "PASS " + password + "\r\n", reply);
    send_ftp_command(connection, "TYPE I\r\n", reply);
    send_ftp_command(connection, "PWD\r\n", reply);

    // Get path without file name
    string full_path = download_source.file_path_on_server;
    string file_name = download_source.file_name;
    size_t file_name_pos = full_path.find(file_name);
    string path = full_path.substr(0, file_name_pos);

    send_ftp_command(connection, "CWD " + path + "\r\n", reply);
  }
}

void FtpDownloader::downloader_trd()
{
  ftp_init();

  string reply;

  for (size_t index = 0; index < connections.size(); ++index) {
    Connection& connection = connections[index];
    pair<string, uint16_t> ip_port_pair;
    if (send_ftp_command(connection, "PASV\r\n", reply))
      ip_port_pair = get_data_ip_port(reply);

    string ip = ip_port_pair.first;
    uint16_t port = ip_port_pair.second;
    open_data_channel(connection, ip, port);

    send_ftp_command(connection, "REST " +
                     to_string(chunks_collection[index].current_pos) + "\r\n",
                     reply);
    send_ftp_command(connection, "RETR " + download_source.file_name + "\r\n",
                     reply);
  }

  fd_set readfds;

  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  static constexpr size_t kBufferLen = 40000;
  char recv_buffer[kBufferLen];

  const size_t kFileSize = writer->get_file_size();

  while (writer->get_total_written_bytes() < kFileSize) {
    int max_fd = 0;
    FD_ZERO(&readfds);
    for (size_t index = 0; index < connections.size(); ++index) {
      size_t current_pos = connections[index].chunk.current_pos;
      size_t end_pos = connections[index].chunk.end_pos;
      // Check if chunk is completed
      if (current_pos < end_pos) {
        int sock_desc = connections[index].ftp_data_sock;
        FD_SET(sock_desc, &readfds);
        max_fd = (max_fd < sock_desc) ? sock_desc : max_fd;
      }
    }

    int sel_retval = select(max_fd + 1, &readfds, NULL, NULL, &timeout);
    // TODO: implement sel_retval == 0
    if (sel_retval < 0)
      cerr << "Select error occured." << endl;
    else if (sel_retval > 0) {
      for (size_t index = 0; index < connections.size(); ++index) {
        int sock_desc = connections[index].ftp_data_sock;
        if (FD_ISSET(sock_desc, &readfds)) {  // Read from the socket
          size_t recvd_bytes = 0;
          ftp_receive_data(connections[index], recv_buffer,  recvd_bytes,
                           kBufferLen);

          // Check if received bytes exceeds end position.
          size_t temp_current_pos = connections[index].chunk.current_pos
                                    + recvd_bytes;
          size_t end_pos = connections[index].chunk.end_pos;
          if (temp_current_pos > end_pos) {
            size_t delta = temp_current_pos - end_pos - 1;
            recvd_bytes -= delta;
          }

          writer->write(recv_buffer, recvd_bytes,
                        connections[index].chunk.current_pos, index);
          connections[index].chunk.current_pos += recvd_bytes;

          connections[index].status = OperationStatus::DOWNLOADING;
        }
        //TODO implement retry
        else {  // the socket timedout
          connections_status[index] = OperationStatus::TIMEOUT;
        }
      }   // End of for loop
    }   // end of 'else if' condition
  }   // End of while loop
}   // End of downloader thread

bool FtpDownloader::send_ftp_command(Connection& connection,
                                     const string& command, string& result)
{
  if (!send_data(connection, command.c_str(), command.length()))
    return false;

  constexpr static size_t kHeaderCapacity = 1000;
  char response[kHeaderCapacity];

  size_t received_bytes = 0;
  while (true) {
    size_t number_of_bytes;

    if (!receive_data(connection, response + received_bytes, number_of_bytes,
          kHeaderCapacity)) {
      cerr << "Socket receive error" << endl;
      break;
    }
    received_bytes += number_of_bytes;

    // RFC9559:
    // A reply is defined to contain the 3-digit code, followed by space.
    string status;
    if (regex_search_string(response, "(\\d{3}\\s)(.*)")) {
      result = string(response, received_bytes);
      return true;
    }
  }
  return false;
}

vector<string> FtpDownloader::split_string(const string& buffer, char delimiter)
{
  stringstream buffer_stream(buffer);
  string token;
  vector<string> splited_strings;
  while (getline(buffer_stream, token, delimiter))
      splited_strings.push_back(token);
  return splited_strings;
}

pair<string, uint16_t> FtpDownloader::get_data_ip_port(const string& buffer)
{
  string ip_port_string;
  regex_search_string(buffer, "(\\d+,\\d+,\\d+,\\d+,\\d+,\\d+)",
                      ip_port_string, 1);

  vector<string> splitted_ip_port = split_string(ip_port_string, ',');

  string ip = splitted_ip_port[0] + "." + splitted_ip_port[1] + "." +
    splitted_ip_port[2] + "." + splitted_ip_port[3];

  string p1 = splitted_ip_port[4];
  string p2 = splitted_ip_port[5];

  uint16_t port = static_cast<uint8_t>(stoi(p1));
  port <<= 8;
  port |= static_cast<uint8_t>(stoi(p2));

  return make_pair(ip, port);
}

void FtpDownloader::open_data_channel(Connection& connection, string ip,
                                      uint16_t port)
{
  URLInfo url_info(ip, port);
  pair<bool, int> result = url_info.get_socket_descriptor();
  if (result.first)
    connection.ftp_data_sock = result.second;
}

bool FtpDownloader::ftp_receive_data(Connection& connection, char* buffer,
    size_t& received_len, size_t buffer_capacity)
{
  received_len = recv(connection.ftp_data_sock, buffer, buffer_capacity, 0);
  if (received_len < 0) {
    connection.status = OperationStatus::SOCKET_RECV_ERROR;
    return false;
  }
  return true;
}
