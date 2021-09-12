#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

#include <map>
#include <vector>
#include <functional>

#include <openssl/bio.h>

#include "thread.h"
#include "file_io.h"
#include "connection.h"
#include "socket_ops.h"
#include "http_proxy.h"
#include "state_manager.h"
#include "request_manager.h"
#include "http_transceiver.h"
#include "info_extractor.h"

using CallBack = std::function<void(size_t)>;

class Downloader : public Thread {
  public:
    const static std::string HTTP_HEADER;

    Downloader(std::unique_ptr<RequestManager> request_manager,
               std::shared_ptr<StateManager> state_manager,
               std::unique_ptr<FileIO> file_io,
               std::unique_ptr<Transceiver> transceiver);

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

    void use_http_proxy(std::string proxy_host, uint16_t proxy_port);

    void register_callback(CallBack callback);

    void set_speed_limit(size_t speed_limit);

    //void set_download_parts(std::queue<std::pair<size_t, Chunk>> initial_parts);

    void set_parts(uint16_t parts);

  private:
    void run() override;
    // Return max_fd
    int set_descriptors();
    void receive_from_connection(size_t index, Buffer& buffer);
    void init_connections();
    void init_connection(uint16_t index);
    void init_connection(bool schedule = false);

    struct RateParams {
      size_t limit = 0;
      size_t speed = 0;
      size_t total_recv_bytes = 0;
      size_t total_limiter_bytes = 0;
      size_t last_overall_recv_bytes = 0;
      std::chrono::steady_clock::time_point last_recv_time_point;
    } rate;

    void rate_process(RateParams& rate, size_t recvd_bytes);

    // <index, socket_ops object>
    void on_dwl_available(uint16_t index,
                          std::unique_ptr<SocketOps> socket_ops);

    void check_new_sock_ops();
    
    fd_set readfds;
    CallBack callback;
    struct timeval timeout;
    time_t timeout_seconds;
    uint16_t number_of_parts;
    std::unique_ptr<FileIO> file_io;
    std::mutex new_available_parts_mutex;
    // <index, connection> [index: same as part index]
    std::unique_ptr<Transceiver> transceiver;
    std::atomic<bool> wait_first_conn_response;
    std::shared_ptr<StateManager> state_manager;
    std::unique_ptr<RequestManager> request_manager;
    struct NewAvailPart {
      uint16_t part_index;
      std::unique_ptr<SocketOps> sock_ops;
    };
    // part index, socket
    std::queue<NewAvailPart> new_available_parts;
    ConnectionManager connection_mngr;
};

#endif
