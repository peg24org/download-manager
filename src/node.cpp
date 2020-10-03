#include <unistd.h>
#include <sys/stat.h>

#include <regex>
#include <chrono>
#include <iomanip>
#include <iostream>

#include "node.h"
#include "url_info.h"
#include "ftp_downloader.h"
#include "http_downloader.h"
#include "https_downloader.h"

using namespace std;

Node::Node(const string& url, const string& optional_path,
           uint16_t number_of_connections, long int timeout)
  : url(url)
  , url_info(url)
  , optional_path(optional_path)
  , number_of_connections(number_of_connections)
  , timeout(timeout)
{
  ++node_index;
}

void Node::run()
{
  // Get download_source
  check_url();
  on_get_file_info(node_index, file_length, download_source.file_name);
  file_path = get_output_path(optional_path, download_source.file_name);

  size_t trd_norm_len = file_length / number_of_connections;

  file_io = make_shared<FileIO>(file_path);

  stat_file_io = make_unique<FileIO>("." + file_path + ".stat");

  download_state_manager =
    make_unique<DownloadStateManager>(move(stat_file_io));

  if (check_resume()) { // Resuming download
    chunks_collection = download_state_manager->get_download_chunks();
    file_io->open();
  }
  else {  // Not resuming download, create chunks collection
    file_io->create(file_length);
    for (int i = 0; i < number_of_connections; i++) {
      size_t start_position = i * trd_norm_len;
      size_t length = trd_norm_len;
      if (i == (number_of_connections - 1)) {
        length = file_length - (trd_norm_len * i);
        start_position = i * trd_norm_len;
      }

      chunks_collection[i].start_pos = start_position==0 ? 0 : start_position+1;
      chunks_collection[i].current_pos = chunks_collection[i].start_pos;
      chunks_collection[i].end_pos = start_position + length;
    }
    download_state_manager->set_initial_state(chunks_collection, file_length);
  }

  unique_ptr<Writer> writer = make_unique<Writer>(file_io,
                                                  download_state_manager);

  downloader = make_downloader(move(writer));

  downloader->start();

	check_download_state();
	downloader->join();
}

void Node::check_url()
{
  while (true) {
    unique_ptr<Downloader> info_downloader = make_downloader();

    // Check if redirected
    string redirected_url;
    if (info_downloader->check_link(redirected_url, file_length)) {
      url = redirected_url;
      cout << "File Length:" << file_length << endl;
      url_info = URLInfo(url);
      cout << "Redirected to:" << url << endl;
    }
    else{
      break;
    }
  }
}

void Node::check_download_state()
{
  size_t total_received_bytes = 0;
  // TODO use downloader status
  while (total_received_bytes < file_length) {
    this_thread::sleep_for(chrono::milliseconds(callback_refresh_interval));
    total_received_bytes = download_state_manager->get_total_written_bytes();
    on_data_received(total_received_bytes);
  }
  // Downloading finished
  download_state_manager->remove_stat_file();
}

unique_ptr<Downloader> Node::make_downloader(unique_ptr<Writer> writer)
{
  unique_ptr<Downloader> downloader_obj;

  vector<int> socket_descriptors;
  for (int i = 0; i < number_of_connections; ++i) {
    pair<bool, int> result = url_info.get_socket_descriptor();
    if (result.first)
      socket_descriptors.push_back(result.second);
    else
      cerr << "Error occured when building downloader." << endl;
  }

  switch(download_source.protocol) {
    case Protocol::HTTP:
      downloader_obj = make_unique<HttpDownloader>(download_source,
                                                   socket_descriptors,
                                                   move(writer),
                                                   chunks_collection,
                                                   timeout);
      break;
    case Protocol::HTTPS:
      downloader_obj = make_unique<HttpsDownloader>(download_source,
                                                    socket_descriptors,
                                                    move(writer),
                                                    chunks_collection,
                                                    timeout);
      break;
    case Protocol::FTP:
      downloader_obj = make_unique<FtpDownloader>(download_source,
                                                  socket_descriptors,
                                                  move(writer),
                                                  chunks_collection,
                                                  timeout);
      break;
  }

  return downloader_obj;
}

std::unique_ptr<Downloader> Node::make_downloader()
{
  vector<int> socket_descriptors;
  pair<bool, int> result = url_info.get_socket_descriptor();
  if (result.first)
    socket_descriptors.push_back(result.second);
  else {
    cerr << "Connection error" << endl;
    exit(1);
  }

  download_source = url_info.get_download_source();
  unique_ptr<Downloader> downloader_obj;
  switch (download_source.protocol) {
    case Protocol::HTTP:
      downloader_obj = make_unique<HttpDownloader>(download_source,
                                                   socket_descriptors);
      break;
    case Protocol::HTTPS:
      downloader_obj = make_unique<HttpsDownloader>(download_source,
                                                    socket_descriptors);
      break;
    case Protocol::FTP:
      downloader_obj = make_unique<FtpDownloader>(download_source,
                                                  socket_descriptors);
      break;
  }

  return downloader_obj;
}

bool Node::check_resume()
{
  stat_file_io =
    make_unique<FileIO>("." + file_path + ".stat");

  if (!(stat_file_io->check_existence()))
    return false;

  return true;
}

string Node::get_output_path(const string& optional_path,
                             const string& source_name)
{
  string path;
  FileIO output_file(optional_path);

  if (output_file.check_existence()) {
    if (output_file.check_path_type() == PathType::DIRECTORY_T) {
      if (optional_path[optional_path.length() - 1] == '/')
        path = optional_path + source_name;
      else
        path = optional_path + "/" + source_name;
    }

    else if (output_file.check_path_type() == PathType::FILE_T)
      path = optional_path;
    else {
      cerr << "Unknown optional path, using default file name." << endl;
      path = source_name;
    }
  }
  else {
    if (!optional_path.empty())
      path = optional_path;
    else
      path = source_name;
  }

  return path;
}

size_t Node::node_index = 0;
