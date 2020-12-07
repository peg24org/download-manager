#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

#include <map>
#include <vector>
#include <functional>

#include <openssl/bio.h>

#include "thread.h"
#include "writer.h"
#include "file_io.h"
#include "socket_ops.h"
#include "http_proxy.h"

using CallBack = std::function<void(size_t)>;

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
    , http_proxy(nullptr)
    , header_skipped(false)
  {
  }

  OperationStatus status;
  struct DownloadChunk chunk;
  BIO* bio;
  SSL* ssl;
  // Used for http, https and ftp command channel.
  std::unique_ptr<SocketOps> socket_ops;
  // Used for ftp media channel.
  std::unique_ptr<SocketOps> ftp_media_socket_ops;
  std::chrono::steady_clock::time_point last_recv_time_point;
  std::string temp_http_header;
  std::unique_ptr<HttpProxy> http_proxy;
  bool header_skipped;
};

class Downloader : public Thread {
  public:
    const static std::string HTTP_HEADER;
    Downloader(const struct DownloadSource& download_source);

    Downloader(const struct DownloadSource& download_source,
               std::unique_ptr<Writer> writer,
               const ChunksCollection& chunks_collection,
               time_t timeout_seconds,
               int number_of_connections=1);

    /**
     * Check the size of file and redirection
     *
     * @param redirect_url: Will be filled with redirected url if redirection
     *                      exist.
     * @param size: Will be filled with size of file if exist.
     *
     * @return 1 if link is redirected otherwise 0, in case of error
     *  it will return -1
     */
    virtual int check_link(std::string& redirect_url, size_t& size) = 0;

    void use_http_proxy(std::string proxy_host, uint16_t proxy_port);

    void register_callback(CallBack callback);

  protected:
    bool regex_search_string(const std::string& input,
                             const std::string& pattern,
                             std::string& output, int pos_of_pattern = 2);
    bool regex_search_string(const std::string& input,
                             const std::string& pattern);

    void run() override;

    virtual bool receive_data(Connection& connection, Buffer& buffer);

    virtual bool send_data(const Connection& connection, const Buffer& buffer);

    virtual bool send_requests() = 0;

    virtual bool send_request(Connection& connection) = 0;
    // Return max_fd
    virtual int set_descriptors() = 0;

    virtual void receive_from_connection(size_t index, Buffer& buffer) = 0;

    virtual bool init_connections();
    virtual bool init_connection(Connection& connection);
    virtual std::vector<int> check_timeout();
    // Retry download in connections
    virtual void retry(const std::vector<int>& connection_indices);

    struct DownloadSource download_source;

    std::unique_ptr<Writer> writer;
    ChunksCollection chunks_collection;
    time_t timeout_seconds;
    std::map<size_t, Connection> connections;
    fd_set readfds;
    int number_of_connections;

  private:
    CallBack callback;
};

#endif
