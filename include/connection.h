#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <vector>
#include <memory>
#include <chrono>

#include "socket_ops.h"
#include "state_manager.h"

enum class OperationStatus {
  ERROR,
  NOT_STARTED,
  DOWNLOADING,
  FINISHED,
  TIMEOUT,
  HTTP_ERROR,
  HTTP_SEL_ERROR,
  FTP_ERROR,
  SSL_ERROR,
  RESPONSE_ERROR,
  SOCKFD_ERROR,
  SOCKET_SEND_ERROR,
  SOCKET_RECV_ERROR,
  SOCKET_CONNECT_ERROR
};

struct Connection {
  Connection() : status(OperationStatus::NOT_STARTED)
    , last_recv_time_point(std::chrono::steady_clock::now())
//    , http_proxy(nullptr)
    , inited(false)
    , request_sent(false)
    , scheduled(false)
    , substitute_created(false)
  {
  }

  enum class Status {
    FAILED,
    SUCCEED,
    REJECTED,
    NEW_PART_NOT_AVAILABLE
  };

  OperationStatus status;
  // Used for http, https and ftp command channel.
  std::unique_ptr<SocketOps> socket_ops;
  // Used for ftp media channel.
  std::unique_ptr<SocketOps> ftp_media_socket_ops;
  std::chrono::steady_clock::time_point last_recv_time_point;
  std::string temp_http_header;
  //std::unique_ptr<HttpProxy> http_proxy;
  bool inited;
  bool request_sent;
  bool scheduled;
  bool substitute_created;
};

class ConnectionManager {
  public:
  ConnectionManager(std::shared_ptr<StateManager> state_manager);
  void set_parts_max(uint16_t parts_max);
  void init();
  std::vector<uint16_t> get_indices_list();
  SocketOps* get_sock_ops(uint16_t index) const;
  void set_sock_ops(std::unique_ptr<SocketOps> socket_ops, uint16_t index);
  ssize_t get_end_pos(uint16_t index) const;
  ssize_t get_start_pos(uint16_t index) const;
  ssize_t get_current_pos(uint16_t index) const;
  bool get_init_stat(uint16_t index) const;
  void set_error(uint16_t index);
  void set_init_stat(bool init_stat, uint16_t index);
  void survey_connections();

  private:
  void generate_one_connection(uint16_t index);
  std::map<uint16_t, Connection> connections;
  std::shared_ptr<StateManager> state_manager;
  std::vector<uint16_t> parts_list;
};

#endif

