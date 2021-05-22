#ifndef _NODE_H
#define _NODE_H

#include <map>

#include "units.h"
#include "thread.h"
#include "file_io.h"
#include "downloader.h"
#include "state_manager.h"

class Node : public Thread {
  public:
    Node(const std::string& url, const std::string& optional_path = kCurrDir,
         uint16_t number_of_parts=1,
         long int timeout=DEFAULT_TIMEOUT_SECONDS);
    virtual void on_get_file_info(size_t node_index, size_t file_size,
                                  const std::string& file_name) {};

    // Set proxy if proxy_url is not empty.
    void set_proxy(std::string proxy_url);

    /**
     * Set speed limit in bytes/second
     *
     * @param speed_limit Speed limit in bytes per second
     */
    void set_speed_limit(size_t speed_limit);

    /**
     * Resume download.
     *
     * @param resume True: for resuming download, False: for new downloading.
     *    Default is False.
     */
    void set_resume(bool resume);

    /**
     * Set number of parts.
     *
     * @param parts Number of parts
     */
    void set_parts(uint16_t parts);

  protected:
    // callback refresh interval in milliseconds
    size_t callback_refresh_interval = 500;

    virtual void on_data_received(size_t received_bytes, size_t speed) = 0;

  private:
    constexpr static char kCurrDir[] = "./";
    constexpr static time_t DEFAULT_TIMEOUT_SECONDS = 10;

    void run();
    void check_download_state();

    // pair of output file path and stat file path.
    std::pair<std::string, std::string> get_output_paths(
        const std::string& file_name);

    void on_data_received_node(size_t speed);

    std::shared_ptr<StateManager> state_manager;

    size_t total_received_bytes;
    static size_t node_index; // index of node

    std::string file_path;
    std::string url;
    const std::string optional_path;
    uint16_t number_of_parts;
    long int timeout;

    std::string proxy_url;
    size_t speed_limit;
    bool resume;
};

#endif
