#ifndef FILE_IO_H
#define FILE_IO_H

#include <mutex>
#include <string>
#include <fstream>

class FileIO {
  public:
    FileIO(std::string file_name);
    /// Creates new file.
    void create(size_t file_length = 0);
    /// Opens existing file.
    void open();
    /**
     * Writes a buffer in file.
     * @param position Position of buffer in file
     * @param length Length of buffer
     * @param buffer Buffer to write in file
     */
    void write(size_t position, size_t length, char* buffer);
    /** Checks the file exists or not.
     * @return The file existence checking result
     */
    bool check_existence();

  private:
    std::mutex io_mutex;
    std::string file_name;
    std::fstream file_stream;
};

#endif
