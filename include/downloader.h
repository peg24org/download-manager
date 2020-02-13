#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

#include "thread.h"
#include "logger.h"
#include "file_io.h"
#include "definitions.h"

using namespace std;

class Downloader : public Thread{
  public:
    Downloader(FileIO& file_io, Logger& logger ,node_struct* node_data_info,
        const struct addr_struct addr_data_info, size_t position,
        size_t trd_length, int trd_index)
    : index(trd_index)
    , trd_len(trd_length)
    , pos(position)
    , file_io(file_io)
    , logger(logger)
    , node_data(node_data_info)
    , addr_data(addr_data_info)
    {};

    void call_node_status_changed(int recieved_bytes, int err_flag = 0);
    void set_index(int value);
    int get_index();
    size_t get_trd_len();

    /**
     * Check the size of file and redirection
     *
     * @param redirect_url: Will be filled with redirected url if redirection
     *                      exist.
     * @param size: Will be filled with size of file if exist.
     *
     * @return True if redirection detected
     */
    virtual bool check_link(string& redirect_url, size_t& size) = 0;
    virtual void connect_to_server() = 0;
    virtual void disconnect() = 0;

    // "(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)";
    const static string HTTP_HEADER;

  protected:
    bool regex_search_string(const string& input, const string& pattern,
      string& output, int pos_of_pattern = 2);
    bool regex_search_string(const string& input, const string& pattern);

    virtual void downloader_trd() = 0;
    virtual bool check_error(int len) const;
    virtual bool socket_send(const char* buffer, size_t len);
    virtual bool socket_receive(char* buffer, size_t& received_len, size_t buffer_capacity);

    int index;
    size_t trd_len;  // file size in bytes
    size_t pos;    // Starting position for download
    FileIO& file_io;
    Logger& logger;
    node_struct* node_data;
    struct addr_struct addr_data;

    void write_to_file(size_t pos, size_t len, char* buf);
    void write_start_pos_log(size_t start_pos);
    int  sockfd = 0;

  private:
    void run();
    bool is_start_pos_written = false;
};

#endif
