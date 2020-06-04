#ifndef _NODE_H
#define _NODE_H

#include <map>

#include "thread.h"
#include "logger.h"
#include "file_io.h"
#include "downloader.h"
#include "definitions.h"

class Node : public Thread {
  public:
    Node(struct addr_struct dwl_str_, int number_of_trds)
       : callback_refresh_interval(DEFAULT_CALL_BACK_REFREASH_INTERVAL)
       , dwl_str(dwl_str_)
       , node_data(dwl_str_.file_name_on_server)
       , file_name(dwl_str_.file_name_on_server)
       , file_io(dwl_str_.file_name_on_server)
       , logger("." + dwl_str_.file_name_on_server + ".LOG")
       , num_of_trds(number_of_trds)
       , file_length(0)
       , total_received_bytes(0)
       , progress(0)
       , speed(0){}

    virtual void on_get_file_stat(size_t node_index, size_t file_size,
        struct addr_struct* addr_data) {};

  protected:
    // callback refresh interval
    size_t callback_refresh_interval;

    struct DownloadChunk {
      size_t start_pos;
      size_t current_pos;
      size_t length;
    };

    virtual void on_data_received(const std::map<int, DownloadChunk>&
        download_chunks) = 0;

  private:
    // Default callback refresh interval in milliseconds
    constexpr static size_t DEFAULT_CALL_BACK_REFREASH_INTERVAL = 1000;
    // <thread index, <start position, length>>
    using DownloadChunks = std::unordered_map<int, std::pair<size_t, size_t>>;
    void wait();
    void run();
    void check_url_details();
    void check_state_of_threads();

    Downloader* create_downloader(int index, size_t length, size_t position);

    //TODO use smart pointer
    std::map<int, Downloader*> download_threads;

    DownloadChunks download_chunks;

    // <index, chunk>
    std::map<int, DownloadChunk> download_chunks_;

    struct addr_struct dwl_str;
    struct node_struct node_data;
    string file_name;
    FileIO file_io;
    Logger logger;

    int num_of_trds; 
    size_t file_length;
    size_t total_received_bytes;
    float progress;
    float speed; // bytes/sec
    static size_t node_index; // index of node
    mutex status_mutex;

    // Checks the downloading file existence and its LOG file.
    bool check_resume();
};

#endif
