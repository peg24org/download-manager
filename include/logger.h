#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <utility>  // pair
#include <unordered_map>

#include "file_io.h"
#include "definitions.h"


/**
 * Log data for each thread:
 * <thread index, <start position, end position>>
 */
using ThreadsLog = std::unordered_map<int, std::pair<size_t, size_t>>;

class Logger {
  public:
    Logger(const std::string& file_name);
    /// Creates new log file and sets file size in log.
    void create(size_t file_size=0);
    /// Opens and read existing log file.
    void open();
    /** 
     * Writes log info.
     * @param thread_index Thread index
     * @param writen_bytes Written bytes in the file
     */
    void write(int thread_index, size_t last_byte);
    /** 
     * Writes log info.
     * @param thread_index Thread index
     * @param first_byte Thread first byte
     * @param writen_bytes Written bytes in the file
     */
    void write_first_position(int thread_index, size_t first_position);
    /**
     * Returns the data for resuming download from log file.
     * @return The threads data that written in the file
     */
    ThreadsLog get_threads_data();
    void remove();

  private:
    void write_log_to_file();

    int file_size;
    FileIO log_file;
    ThreadsLog threads_log;
};

#endif
