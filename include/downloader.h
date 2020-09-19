#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

#include <vector>

#include <openssl/bio.h>

#include "thread.h"
#include "writer.h"
#include "url_info.h"
#include "file_io.h"
#include "download_state_manager.h"

enum class OperationStatus {
  ERROR,
  NOT_STARTED,
  DOWNLOADING,
  FINISHED,
  TIMEOUT,
  NO_ERROR,
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

struct StatusStruct {
  StatusStruct(): operation_status(OperationStatus::NOT_STARTED),
                  error_value(0) {}

  OperationStatus operation_status;
  int error_value;
};

struct Connection {
  Connection() : sock_desc(0), status(OperationStatus::NOT_STARTED),
                 bio(nullptr) {}

  int sock_desc;
  int ftp_data_sock;
  OperationStatus status;
  struct DownloadChunk chunk;
  BIO* bio;
  SSL* ssl;
};

class Downloader : public Thread {
  public:
    const static string HTTP_HEADER;
    Downloader(const struct DownloadSource& download_source,
               const std::vector<int>& socket_descriptors);

    Downloader(const struct DownloadSource& download_source,
               const std::vector<int>& socket_descriptors,
               std::unique_ptr<Writer> writer,
               const ChunksCollection& chunks_collection,
               time_t timeout_interval=DEFAULT_TIMEOUT_SECONDS);

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
    virtual int check_link(string& redirect_url, size_t& size) = 0;

  protected:
    bool regex_search_string(const string& input, const string& pattern,
                             string& output, int pos_of_pattern = 2);
    bool regex_search_string(const string& input, const string& pattern);

    virtual void downloader_trd() = 0;
    
    virtual bool receive_data(Connection& connection, char* buffer,
                              size_t& received_len, size_t buffer_capacity);

    virtual bool send_data(Connection& connection, const char* buffer,
                           size_t len);

    struct DownloadSource download_source;

    std::unique_ptr<Writer> writer;
    ChunksCollection chunks_collection;
    std::vector<int> socket_descriptors;
    struct timeval timeout_interval;
    std::map<size_t, Connection> connections;

  private:
    constexpr static time_t DEFAULT_TIMEOUT_SECONDS = 5;
    static DownloadStateManager* download_state_manager;

    void run() override;
};

#endif
