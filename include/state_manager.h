#ifndef _STATE_MANAGER_H
#define _STATE_MANAGER_H

#include <map>
#include <cmath>
#include <queue>
#include <memory>
#include <iostream>

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
     * @param file_name Downloading file name.
     */
    StateManager(const std::string& file_path);

    /**
     * Check stat existence.
     *
     * @return True if .stat file is available.
     */
    bool state_file_available() const;

    bool part_available() const;

    std::pair<size_t, Chunk> get_part();

    void create_new_state(size_t file_size);

    void set_chunk_size(size_t chunk_size);

    size_t get_chunk_size() const;

    size_t get_file_size() const;

    size_t get_total_recvd_bytes() const;

    std::queue<std::pair<size_t, Chunk>> get_initial_parts() const;

    void retrieve();
    /**
     *  Updates status of some chunk.
     *
     *  @param index Index of updating chunk.
     *  @param recvd_butes Received bytes for chunk.
     */
    void update(size_t index, size_t recvd_butes);

  protected:
    std::unique_ptr<FileIO> state_file;

  private:
    constexpr static size_t kMinChunkSize = 20_MB;
    void read_raw_data();
    void store();
    void remove_finished_parts();

    // <index, chunk>
    std::map<size_t, Chunk> parts;
    std::queue<std::pair<size_t, Chunk>> initial_parts;
    size_t initial_index;
    size_t download_file_size;
    size_t total_recvd_bytes;
    size_t chunk_size;
    bool inited;
};

#endif
