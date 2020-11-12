#ifndef _TEST_UTILS_H
#define _TEST_UTILS_H

#include <memory>
#include <exception>

#include "file_io.h"

class FileIOMock : public FileIO
{
  public:
    FileIOMock() : FileIO("NEVER_CREATING_FILE_NAME"), file_opened(false) {}
    void create(size_t file_length = 0) override;
    void open() override;
    void write(const char* buffer, size_t length, size_t position=0) override;
    std::string get_file_contents() override;

    void remove() override;

  private:
    std::string file_contents;
    bool file_opened;
};

std::unique_ptr<char[]> get_random_buffer(size_t length);

bool buffer_cmp(char* buffer_1, char* buffer_2, size_t len);

#endif
