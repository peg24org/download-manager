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
#include "connection_manager.h"

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

    // TODO: remove empty function.
    virtual int check_link(std::string& redirect_url, size_t& size) {};

    void use_http_proxy(std::string proxy_host, uint16_t proxy_port);

    void register_callback(CallBack callback);

    void set_speed_limit(size_t speed_limit);

    void set_download_parts(std::queue<std::pair<size_t, Chunk>> initial_parts);

    void set_parts(uint16_t parts);

  protected:
    bool regex_search_string(const std::string& input,
                             const std::string& pattern,
                             std::string& output, int pos_of_pattern = 2);

    bool regex_search_string(const std::string& input,
                             const std::string& pattern);

    void run() override;

    virtual bool receive_data(Connection& connection, Buffer& buffer);

    virtual bool send_data(const Connection& connection, const Buffer& buffer);

    // TODO: remove all empty functions.
    virtual bool send_requests() {};

    virtual bool send_request(Connection& connection) {};
    // Return max_fd
    virtual int set_descriptors();

    virtual void receive_from_connection(size_t index, Buffer& buffer);

    virtual void init_connections();

    void init_connection();

    virtual Connection::Status create_connection(bool info_connection=false);

    virtual std::vector<int> check_timeout();
    // Retry download in connections
    virtual void retry(const std::vector<int>& connection_indices);

    std::unique_ptr<FileIO> file_io;
    std::shared_ptr<StateManager> state_manager;
    time_t timeout_seconds;
    // <index, connection> [index: same as part index]
    std::map<size_t, Connection> connections;
    fd_set readfds;
    uint16_t number_of_parts;

  private:
    std::unique_ptr<RequestManager> request_manager;
    CallBack callback;
    // <index, chunk>
    std::queue<std::pair<size_t, Chunk>> initial_parts;

    struct NewAvailPart {
      uint16_t part_index;
      std::unique_ptr<SocketOps> sock_ops;
    };
    // part index, socket
    std::mutex new_available_parts_mutex;
    std::queue<NewAvailPart> new_available_parts;

    struct RateParams {
      size_t limit = 0;
      size_t speed = 0;
      size_t total_recv_bytes = 0;
      size_t total_limiter_bytes = 0;
      size_t last_overall_recv_bytes = 0;
      std::chrono::steady_clock::time_point last_recv_time_point;
    } rate;

    void rate_process(RateParams& rate, size_t recvd_bytes);

    void update_connection_stat(size_t recvd_bytes, size_t index);

    void survey_connections();

    // <index, socket_ops object>
    void on_dwl_available(uint16_t index,
                          std::unique_ptr<SocketOps> socket_ops);

    void check_new_sock_ops();
    
    std::unique_ptr<Transceiver> transceiver;
    struct timeval timeout;
    std::atomic<bool> wait_first_conn_response;
};

#endif
