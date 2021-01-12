#ifndef _TEST_UTILS_H
#define _TEST_UTILS_H

#include <memory>
#include <vector>
#include <exception>

#include "file_io.h"

class FileIOMock : public FileIO
{
  public:
    FileIOMock();
    virtual void create(size_t file_length = 0) override;
    void open() override;
    void write(const char* buffer, size_t length, size_t position=0) override;
    char* get_file_buffer();
    bool check_existence() const override;
    void set_existence(bool input);
    std::string get_file_contents() override;

  protected:
    std::string file_contents;
    bool file_opened;
    bool existence;

  private:
    std::unique_ptr<char[]> file_buffer;
};

class StatFileIOMock : public FileIOMock
{
  public:
    using FileIOMock::FileIOMock;
    void write(const char* buffer, size_t length, size_t position=0) override;
    std::string get_file_contents() override;
};

std::unique_ptr<char[]> get_random_buffer(size_t length, char start_char=0,
                                          char end_char=255);
std::string get_random_string(size_t length);

#endif
