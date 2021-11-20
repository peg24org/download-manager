#include "ftp_request_manager.h"

#include <sstream>
#include <cstring>
#include <iostream>
#include <algorithm>

#include "ftp_transceiver.h"

using namespace std;

void FtpRequestManager::send_requests()
{
  Request request;
  {
    if (requests.size() == 0)
      return;
    lock_guard<mutex> lock(request_mutex);
    request = requests.begin()->second;;
    requests.erase(requests.begin());
  }

  unique_ptr<SocketOps> sock_ops = info_extractor->acquire_sock_ops();
  initialize(sock_ops.get(), info_extractor->get_path());
  string ip_port_str = get_data_channel_addr(sock_ops.get());
  pair<string, uint16_t> ip_port = get_ip_port_pair(ip_port_str);
  unique_ptr<SocketOps> data_sock_ops = open_data_channel(ip_port.first,
                                                          ip_port.second);
  if (send_ftp_requst(request, sock_ops.get()))
    notify_dwl_available(request.request_index, move(data_sock_ops));
}

string FtpRequestManager::get_data_channel_addr(SocketOps* sock_ops)
{
  Buffer command(string("PASV\r\n"));
  pair<bool, string> result = send_ftp_command(command, sock_ops);
  if (result.first != true)
    cerr << "sending ftp command failed." << endl;

  return result.second;
}

pair<string, uint16_t> FtpRequestManager::get_ip_port_pair(const string& buffer)
{
  const size_t sub_start = buffer.find('(') + 1;
  const size_t sub_len = buffer.find(')') - sub_start;
  string ip_port_str(buffer.substr(sub_start, sub_len));

  replace(ip_port_str.begin(), ip_port_str.end(), ',', ' ');

  stringstream address_stream(ip_port_str);

  string ip;
  string dot = "";
  for (uint8_t index = 0; index < 4; ++index) {
    string temp_str;
    address_stream >> temp_str;
    ip += dot + temp_str;
    dot = ".";
  }

  uint16_t port_bytes[2];
  address_stream >> port_bytes[0];
  address_stream >> port_bytes[1];

  uint16_t port = static_cast<uint8_t>(port_bytes[0]);
  port <<= 8;
  port |= static_cast<uint8_t>(port_bytes[1]);

  return make_pair(ip, port);
}

unique_ptr<SocketOps> FtpRequestManager::open_data_channel(const string& ip,
                                                           uint16_t port)
{
  unique_ptr<SocketOps> sock_ops = make_unique<SocketOps>(ip, port);
  sock_ops->connect();

  return sock_ops;
}

bool FtpRequestManager::initialize(SocketOps* sock_ops, const string& dir)
{
  static_cast<FtpTransceiver*>(transceiver.get())->send_init_commands(sock_ops);

  Buffer command(string("CWD ") + dir + "\r\n");
  pair<bool, string> result = send_ftp_command(command, sock_ops);
  if (result.first != true)
    cerr << "sending ftp command failed." << endl;

  return result.first;
}

bool FtpRequestManager::send_ftp_requst(const Request& request,
                                        SocketOps* sock_ops)
{
  bool result = true;
  const array<Buffer, 2> commands = {
    Buffer(string("REST " + to_string(request.start_pos) + "\r\n")),
    Buffer(string("RETR " + info_extractor->get_file_name()+ "\r\n"))
  };

  for (Buffer command : commands) {
    pair<bool, string> response = send_ftp_command(command, sock_ops);
    result &= response.first;
    const uint16_t kResponseCode = stoi(response.second.substr(0, 3));
    static constexpr uint16_t kFileStatOk = 150;
    static constexpr uint16_t kReqPosAcceppted = 350;
    if (kResponseCode != kFileStatOk && kResponseCode != kReqPosAcceppted)
      result = false;
  }
  return result;
}

pair<bool, string> FtpRequestManager::send_ftp_command(const Buffer& command,
                                                       SocketOps* sock_ops)
{
  string response("");
  bool result = true;
  if (!transceiver->send(command, sock_ops)) {
    cerr << "Ftp command sending error: " << "reply" << endl;
    result = false;
  }
  else {
    Buffer recv_buffer;
    while (true) {
      if (!transceiver->receive(recv_buffer, sock_ops)) {
        cerr << "Ftp receive error" << endl;
        result = false;
        break;
      }
      // RFC9559:
      // A reply is defined to contain the 3-digit code, followed by space.
      // string response_code_str(recv_buffer, 3);
      if (strstr(recv_buffer, "\r\n") != nullptr) {
        if (response == "") {
          response = string(recv_buffer, recv_buffer.length());
          break;
        }
      }
    }
  }

  return make_pair(result, response);
}

