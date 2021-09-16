#include <unistd.h>
#include <sys/stat.h>

#include <regex>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <functional>

#include "node.h"
#include "info_extractor.h"
#include "ftp_request_manager.h"
#include "http_request_manager.h"
#include "ftp_transceiver.h"
#include "http_transceiver.h"
#include "https_transceiver.h"

using namespace std;

Node::Node(const string& url, const string& optional_path,
           uint16_t number_of_parts, long int timeout)
  : url(url)
  , optional_path(optional_path)
  , number_of_parts(number_of_parts)
  , timeout(timeout)
  , resume(false)
{
  ++node_index;
}

void Node::run()
{
  unique_ptr<InfoExtractor> connection_manager;
  connection_manager = make_unique<InfoExtractor>(url);

  const pair<string, string> paths= get_output_paths(
      connection_manager->get_file_name());
  const size_t file_length = connection_manager->get_file_length();
  const string& file_name = paths.first;

  on_get_file_info(node_index, file_length, file_name);

  unique_ptr<FileIO> file_io = make_unique<FileIO>(paths.first);

  unique_ptr<FileIO> stat_file_io = make_unique<FileIO>(paths.second);

  state_manager = make_shared<StateManager>(paths.second);
  bool state_file_available = state_manager->state_file_available();
  bool main_file_available = file_io->check_existence();

  if (resume && state_file_available && main_file_available) { // Resuming download
    state_manager->retrieve();
    file_io->open();
  }
  else {  // Not resuming download, create chunks collection
    file_io->create(file_length);
    state_manager->create_new_state(file_length);
  }

  state_manager->set_chunks_num(number_of_parts);

  // Create and register callback
  CallBack callback = bind(&Node::on_data_received_node, this,
                           placeholders::_1);

  unique_ptr<RequestManager> request_manager;

  Protocol protocol = connection_manager->get_protocol();
  unique_ptr<Transceiver> transceiver;

  switch (protocol) {
    case Protocol::HTTP:
      request_manager = make_unique<HttpRequestManager>(move(connection_manager),
                                                    make_unique<HttpTransceiver>());
      transceiver = make_unique<HttpTransceiver>();
      break;
    case Protocol::HTTPS:
      request_manager = make_unique<HttpRequestManager>(move(connection_manager),
                                                    make_unique<HttpsTransceiver>());
      transceiver = make_unique<HttpsTransceiver>();
      break;
    case Protocol::FTP:
      request_manager = make_unique<FtpRequestManager>(move(connection_manager),
                                                    make_unique<FtpTransceiver>());
      transceiver = make_unique<FtpTransceiver>();
      break;
  }

  downloader = make_unique<Downloader>(move(request_manager), state_manager,
                                       move(file_io), move(transceiver));
  downloader->register_callback(callback);
  downloader->set_parts(number_of_parts);
  downloader->set_speed_limit(speed_limit);

  downloader->start();
  downloader->join();
}

pair<string, string> Node::get_output_paths(const string& file_name)
{
  pair<string, string> result;
  FileIO output_file(optional_path);
  string main_file_path;
  string stat_file_path;

  if (output_file.check_existence()) {
    if (output_file.check_path_type() == PathType::DIRECTORY_T) {
      if (optional_path[optional_path.length() - 1] == '/') {
        result.first = optional_path + file_name;
        result.second = optional_path + "." + file_name;
      }
      else {
        result.first = optional_path + "/" + file_name;
        result.second = optional_path + "/" + "." + file_name;
      }
    }

    else if (output_file.check_path_type() == PathType::FILE_T) {
      result.first = optional_path;
      result.first = "." + optional_path;
    }
    else {
      cerr << "Unknown path not exist, using default file name." << endl;
      result.first = file_name;
      result.second = "." + file_name;
    }
  }
  else {
    if (!optional_path.empty()) {
      result.first = optional_path;
      result.second = "." + optional_path;
    }
    else {
      result.first = file_name;
      result.second = "." + file_name;
    }
  }
  result.second += ".stat";

  return result;
}

void Node::set_proxy(string proxy_url)
{
  this->proxy_url = proxy_url;
}

void Node::set_speed_limit(size_t speed_limit)
{
  this->speed_limit = speed_limit;
}

void Node::set_resume(bool resume)
{
  this->resume = resume;
}

void Node::set_parts(uint16_t parts)
{
  number_of_parts = parts;
}

void Node::pause_download()
{
  downloader->pause_download();
}

void Node::resume_download()
{
  downloader->resume_download();
}

void Node::on_data_received_node(size_t speed)
{
  size_t total_received_bytes = 0;
  //total_received_bytes = download_state_manager->get_total_written_bytes();
  total_received_bytes = state_manager->get_total_recvd_bytes();
  on_data_received(total_received_bytes, speed);
}

size_t Node::node_index = 0;
