#ifndef _TEST_UTILS
#define _TEST_UTILS

#include "file_io.h"

#include <exception>

class FileIOMock : public FileIO
{
  public:
    FileIOMock() : FileIO("NEVER_CREATING_FILE_NAME"), file_opened(false) {}

    void create(size_t file_length = 0) override {
      file_contents = std::string(file_length, '\0');
      file_opened = true;
    }

    void open() override {
      file_opened = true;
    }

    void write(const char* buffer, size_t length, size_t position=0) override {
      file_contents.replace(position, length, buffer);
    }

    std::string get_file_contents() override {
      if (!file_opened)
        throw std::runtime_error("FileIO is not open.");
      return file_contents;
    }

    void remove() override {}

  private:
    std::string file_contents;
    bool file_opened;
};

#endif
