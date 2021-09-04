#ifndef _STATE_MANAGER_H
#define _STATE_MANAGER_H

#include <map>
#include <cmath>
#include <queue>
#include <memory>
#include <iostream>
#include <unordered_map>

#include "units.h"
#include "file_io.h"

struct Chunk {
  Chunk();
  Chunk(size_t start, size_t current, size_t end);
  bool operator==(const Chunk& rhs) const;

  size_t start;
  size_t current;
  size_t end;
  bool finished;
  bool busy;
};

class StateManager
{
  public:
    ~StateManager();
    /**
     * Ctor.
     *
     * @param file_path Downloading file path.
     */
    StateManager(const std::string& file_path,
                 size_t chunk_len = kDefaultMinChunkLen);

    void retrieve();
    void generate_part();
    size_t get_file_size() const;
    size_t get_chunks_max() const;
    size_t get_chunks_num() const;
    bool state_file_available() const;
    uint16_t get_chunks_num_max() const;
    size_t get_total_recvd_bytes() const;
    void create_new_state(size_t file_size);
    std::vector<uint16_t> get_parts() const;
    void set_chunks_num(uint16_t chunks_num);
    size_t get_end_pos(uint16_t index) const;
    size_t get_start_pos(uint16_t index) const;
    size_t get_current_pos(uint16_t index) const;
    void update(size_t index, size_t recvd_butes);

  private:
    static constexpr size_t kDefaultMinChunkLen = 10_MB;
    void store();
    void read_raw_data();
    void generate_parts();
    void remove_finished_parts();

    std::unique_ptr<FileIO> state_file;
    // <index, chunk>
    std::unordered_map<uint16_t, Chunk> parts;
    std::queue<std::pair<size_t, Chunk>> initial_parts;
    size_t initial_index;
    size_t file_size;
    size_t total_recvd_bytes;
    size_t chunk_len;
    uint16_t chunks_num;
    bool inited;
    uint16_t chunks_num_max;
};

#endif
