#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

#include "thread.h"
#include "logger.h"
#include "file_io.h"
#include "definitions.h"

using namespace std;

enum class OperationStatus {
  NOT_STARTED,
  DOWNLOADING,
  FINISHED,
  ERROR,
  NO_ERROR,
  HTTP_ERROR,
  FTP_ERROR,
  SSL_ERROR,
  RESPONSE_ERROR,
  SOCKFD_ERROR,
  SOCKET_SEND_FUNCTION_ERROR,
  SOCKET_RECV_FUNCTION_ERROR,
  SOCKET_CONNECT_FUNCTION_ERROR
};

struct StatusStruct {
  StatusStruct():
    operation_status(OperationStatus::NOT_STARTED),
    error_value(0) {}
  OperationStatus operation_status;
  int error_value;
};

class Downloader : public Thread {
  public:
    Downloader(FileIO& file_io, Logger& logger ,node_struct* node_data_info,
        const struct addr_struct addr_data_info, size_t position,
        size_t trd_length, int trd_index,
        time_t timeout_interval=DEFAULT_TIMEOUT_SECONDS)
    : index(trd_index)
    , trd_len(trd_length)
    , pos(position)
    , file_io(file_io)
    , logger(logger)
    , node_data(node_data_info)
    , addr_data(addr_data_info)
    , sockfd(0)
    , total_received_bytes(0)
    , timeout_interval({.tv_sec=timeout_interval, .tv_usec=0})
    , status(StatusStruct())
    {};

    void call_node_status_changed(int recieved_bytes,
        StatusStruct status=StatusStruct());
    void set_index(int value);
    int get_index();
    size_t get_trd_len();
    size_t get_current_pos() const;

    /// Get the status of downloader thread
    StatusStruct get_status() const;

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
    virtual void disconnect() = 0;

    // "(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)";
    const static string HTTP_HEADER;

  protected:
    bool regex_search_string(const string& input, const string& pattern,
      string& output, int pos_of_pattern = 2);
    bool regex_search_string(const string& input, const string& pattern);
    bool connection_init();

    virtual void downloader_trd() = 0;
    virtual bool check_error(int len) const;
    virtual bool socket_send(const char* buffer, size_t len);
    virtual bool socket_receive(char* buffer, size_t& received_len,
        size_t buffer_capacity);
    void set_status(OperationStatus operation_status, int error_value=0);
    int index;
    size_t trd_len;  // file size in bytes
    size_t pos;    // Starting position for download
    FileIO& file_io;
    Logger& logger;
    node_struct* node_data;
    struct addr_struct addr_data;

    void write_to_file(size_t pos, size_t len, char* buf);
    void write_start_pos_log(size_t start_pos);
    int  sockfd;
    fd_set readfds;
    size_t total_received_bytes;

  private:
    constexpr static time_t DEFAULT_TIMEOUT_SECONDS = 5;
    struct timeval timeout_interval;

    void run() override;

    bool is_start_pos_written = false;
    StatusStruct status;
};

#endif
