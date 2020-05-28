#ifndef _DEFINITIONS_H
#define _DEFINITIONS_H

#include <mutex>

struct addr_struct{
  int port;
  std::string ip;
  std::string file_path_on_server;
  std::string file_name_on_server;
  std::string host_name;
  bool encrypted;
  int protocol;    // Enum protocol type
};

enum protocol_type {
  kHttp,
  kFtp,
};

class Node;

struct node_struct {
  node_struct(const std::string& file_name) :
    file_name(file_name) {}

  // node_struct must not be copied
  node_struct(const node_struct& other) = delete;

  FILE* log_fp;
  Node* node;
  bool resuming;
  std::string log_buffer_str;
  std::string file_name;
  std::mutex node_mutex;
};

constexpr size_t CHUNK_SIZE = 256 * 1024;

#endif
