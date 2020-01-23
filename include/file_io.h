#ifndef FILE_IO_H
#define FILE_IO_H

#include <string>
#include <cstdio>
#include <mutex>

class FileIO {
  public:
    FileIO(std::string file_name);
    // Creates new file
    void create(size_t file_length);
    // Opens existing file
    void open();
    // Writes buffer 'buf' in position 'pos'
    void write(size_t pos, size_t len, char* buf);

  private:
    std::string file_name;
    std::mutex io_mutex;
    FILE* file_pointer;
};

#endif
