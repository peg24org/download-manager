#ifndef _DOWNLOAD_STATE_H
#define _DOWNLOAD_STATE_H

#include <mutex>
#include <memory>
#include <iostream>
#include <unordered_map>

#include <file_io.h>

struct DownloadChunk {
  size_t start_pos;
  size_t current_pos;
  size_t end_pos;

  bool operator==(const DownloadChunk& rhs) const {
    return start_pos == rhs.start_pos && current_pos == rhs.current_pos &&
      end_pos == rhs.end_pos;
  }
};

using ChunksCollection = std::unordered_map<size_t, DownloadChunk>;

class DownloadStateManager
{
  public:
    DownloadStateManager(std::unique_ptr<FileIO> file_io);

    /**
     * Sets initial state of downloading
     *
     * @param chunks_collection Collection of chunks for initialization
     * @param file_size File size to store
     */
    void set_initial_state(const ChunksCollection& chunks_collection,
                           size_t file_size);

    /**
     * Updates download chunk in some index
     * 
     * @index Index of chunk for update its current position
     * @position Position of last written byte in the file
     */
    void update(size_t index, size_t current_pos);

    /**
     * Retrieves saved download state from file.
     * It used when download resumes.
     *
     * @return Download chunks collection
     */
    ChunksCollection get_download_chunks();
    /**
     * Gets chunks collection in downloading state.
     *
     * @return Download chunks collection
     */
    ChunksCollection get_chunks_collection() const;

    size_t get_file_size() const;

    size_t get_total_written_bytes() const;

    void remove_stat_file();

  protected:
    std::unique_ptr<FileIO> file_io;

  private:
    void store();
    void retrieve();

    ChunksCollection chunks_collection;
    size_t file_size;
    mutable std::mutex chunks_collection_mutex;
};

#endif
