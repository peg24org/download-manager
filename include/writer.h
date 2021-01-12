#ifndef _WRITER_H
#define _WRITER_H

#include <memory>

#include "buffer.h"
#include "file_io.h"
#include "state_manager.h"

class Writer
{
  public:
  Writer(std::shared_ptr<FileIO> file_io);

  void write(const char* buffer, size_t length, size_t position);

  void write(const Buffer& buffer,size_t position);

  size_t get_file_size() const;

  size_t get_total_written_bytes() const;

  private:
  std::shared_ptr<FileIO> file_io;
};

#endif
