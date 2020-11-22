#include "ftp_downloader.h"

#include <regex>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <iostream>

#include <arpa/inet.h>

#include "node.h"

using namespace std;

FtpDownloader::FtpDownloader(const struct DownloadSource& download_source)
  : Downloader(download_source)
{
}

FtpDownloader::FtpDownloader(const struct DownloadSource& download_source,
                             std::unique_ptr<Writer> writer,
                             ChunksCollection& chunks_collection,
                             long int timeout,
                             int number_of_connections)
  : Downloader(download_source, move(writer), chunks_collection, timeout,
               number_of_connections)
{
  for (auto chunk : chunks_collection) {
    connections[chunk.first].chunk = chunk.second;
    connections[chunk.first].status = OperationStatus::NOT_STARTED;
  }
}

int FtpDownloader::check_link(string& redirected_url, size_t& size)
{
  if (!init_connections())
    return -1;
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
  const vector<string> init_commands = {
    "",
    "USER " + username + "\r\n",
    "PASS " + password + "\r\n",
    "TYPE I\r\n",
    "PWD\r\n"
  };

  string reply;
  for (size_t index = 0; index < connections.size(); ++index) {
    Connection& connection = connections[index];

    for (const auto& command : init_commands)
      if (!send_ftp_command(connection, command, reply))
        cerr << "Error occurred: " << reply << endl;

    string path = download_source.file_path;
    string file_name = download_source.file_name;

    const string kCwdCommand = "CWD " + path + "\r\n";
    send_ftp_command(connection, kCwdCommand, reply);
  }
}

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

void FtpDownloader::open_data_channel(Connection& connection, const string& ip,
                                      uint16_t port)
{
  connection.ftp_media_socket_ops = make_unique<SocketOps>(ip, port);
  connection.ftp_media_socket_ops->connect();
}

bool FtpDownloader::send_requests()
{
  ftp_init();

  string reply;

  for (int index = 0; index < number_of_connections; ++index) {
    Connection& connection = connections[index];
    pair<string, uint16_t> ip_port_pair;
    if (send_ftp_command(connection, "PASV\r\n", reply))
      ip_port_pair = get_data_ip_port(reply);
    else
      cerr << "Error occurred: " << reply << endl;

    string ip = ip_port_pair.first;
    uint16_t port = ip_port_pair.second;
    open_data_channel(connection, ip, port);

    const size_t kCurrentPos = chunks_collection[index].current_pos;
    const string kRestCommand = "REST " + to_string(kCurrentPos) + "\r\n";
    if (!send_ftp_command(connection, kRestCommand, reply))
      cerr << "Error occurred: " << reply << endl;

    const string kRetrCommand = "RETR " + download_source.file_name + "\r\n";
    if (!send_ftp_command(connection, kRetrCommand, reply))
      cerr << "Error occurred: " << reply << endl;
  }
}

int FtpDownloader::set_descriptors()
{
  int max_fd = 0;
  FD_ZERO(&readfds);
  for (size_t index = 0; index < connections.size(); ++index) {
    const Connection& connection = connections[index];
    size_t current_pos = connection.chunk.current_pos;
    size_t end_pos = connection.chunk.end_pos;
    // Check if chunk is completed
    if (current_pos < end_pos) {
      int sock_desc = connection.ftp_media_socket_ops->get_socket_descriptor();
      FD_SET(sock_desc, &readfds);
      max_fd = (max_fd < sock_desc) ? sock_desc : max_fd;
    }
  }

  return max_fd;
}

size_t FtpDownloader::receive_from_connection(size_t index, char* buffer,
                                              size_t buffer_capacity)
{
  size_t recvd_bytes = 0;
  Connection& connection = connections[index];
  int sock_desc = connection.ftp_media_socket_ops->get_socket_descriptor();

  if (FD_ISSET(sock_desc, &readfds)) {  // read from the socket
    ftp_receive_data(connection, buffer,  recvd_bytes, buffer_capacity);

    // Correct last received part for each chunk.
    size_t end_pos = connection.chunk.end_pos;
    size_t current_pos = connection.chunk.current_pos;
    int64_t redundant_bytes = current_pos + recvd_bytes - end_pos;
    if (redundant_bytes > 0)
      recvd_bytes -= redundant_bytes - 1;
  }

  return recvd_bytes;
}


bool FtpDownloader::receive_data(Connection& connection, char* buffer,
                                 size_t& recv_len, size_t buffer_capacity)
{
  recv_len = recv(connection.socket_ops->get_socket_descriptor(), buffer,
                  buffer_capacity, 0);
  if (recv_len < 0) {
    connection.status = OperationStatus::SOCKET_RECV_ERROR;
    return false;
  }

  return true;
}

bool FtpDownloader::ftp_receive_data(Connection& connection, char* buffer,
                                     size_t& received_len,
                                     size_t buffer_capacity)
{
  received_len = recv(connection.ftp_media_socket_ops->get_socket_descriptor(),
                      buffer, buffer_capacity, 0);
  if (received_len < 0) {
    connection.status = OperationStatus::SOCKET_RECV_ERROR;
    return false;
  }

  return true;
}
