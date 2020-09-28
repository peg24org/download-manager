#ifndef _NODE_H
#define _NODE_H

#include <map>

#include "thread.h"
#include "file_io.h"
#include "url_info.h"
#include "downloader.h"
#include "download_state_manager.h"

class Node : public Thread {
  public:
    Node(const std::string& url, const std::string& file_name,
         uint16_t number_of_connections=1);
    virtual void on_get_file_info(size_t node_index, size_t file_size,
                                  const std::string& file_name) {};

  protected:
    // callback refresh interval in milliseconds
    size_t callback_refresh_interval = 500;

    virtual void on_data_received(size_t received_bytes) = 0;

  private:
    void build_downloader(std::unique_ptr<Writer> writer);
    void run();
    void check_url();
    void check_download_state();
    // Checks the downloading file existence and its LOG file.
    bool check_resume();

    std::unique_ptr<Downloader> downloader;
    ChunksCollection chunks_collection;
    std::shared_ptr<DownloadStateManager> download_state_manager;

    std::shared_ptr<FileIO> file_io;
    std::unique_ptr<FileIO> stat_file_io;

    size_t file_length;
    size_t total_received_bytes;
    static size_t node_index; // index of node

    std::string url;
    URLInfo url_info;
    std::string file_name;
    uint16_t number_of_connections;
    struct DownloadSource download_source;
};

#endif
