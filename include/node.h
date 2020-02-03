#ifndef _NODE_H
#define _NODE_H

#include <map>

#include "thread.h"
#include "logger.h"
#include "file_io.h"
#include "downloader.h"
#include "definitions.h"

class Node:public Thread {
  public:
    Node(struct addr_struct dwl_str_, int number_of_trds) : dwl_str(dwl_str_)
       , node_data(dwl_str_.file_name_on_server)
       , file_name(dwl_str_.file_name_on_server)
       , file_io(dwl_str_.file_name_on_server)
       , logger("." + dwl_str_.file_name_on_server + ".LOG")
       , num_of_trds(number_of_trds)
       , file_length(0)
       , total_received_bytes(0)
       , progress(0)
       , speed(0){}

    void on_get_status(struct addr_struct* addr_data,int downloader_trd_index,
        size_t total_trd_len, size_t received_bytes, int stat_flag);
    virtual void on_status_changed(int downloader_trd_index,
        size_t total_trd_len, size_t received_bytes,
        struct  addr_struct* addr_data){};
    virtual void on_get_file_stat(size_t node_index, size_t file_size,
        struct addr_struct* addr_data) {};

  private:
    // <thread index, <start position, length>>
    using DownloadChunks = std::unordered_map<int, std::pair<size_t, size_t>>;
    void wait();
    void run();
    void check_url_details();

    std::map<int, Downloader*> download_threads;

    DownloadChunks download_chunks;

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

    // Checks the downloading file existence and its LOG file.
    bool check_resume();
};

#endif
