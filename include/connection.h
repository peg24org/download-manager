#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <memory>
#include <chrono>

#include <openssl/bio.h>

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
    , bio(nullptr)
    , ssl(nullptr)
    , last_recv_time_point(std::chrono::steady_clock::now())
//    , http_proxy(nullptr)
    , header_skipped(false)
    , inited(false)
    , request_sent(false)
  {
  }

  enum class Status {
    FAILED,
    SUCCEED,
    REJECTED,
    NEW_PART_NOT_AVAILABLE
  };

  OperationStatus status;
  Chunk chunk;
  BIO* bio;
  SSL* ssl;
  // Used for http, https and ftp command channel.
  std::unique_ptr<SocketOps> socket_ops;
  // Used for ftp media channel.
  std::unique_ptr<SocketOps> ftp_media_socket_ops;
  std::chrono::steady_clock::time_point last_recv_time_point;
  std::string temp_http_header;
  //std::unique_ptr<HttpProxy> http_proxy;
  bool header_skipped;
  bool inited;
  bool request_sent;
};

#endif

