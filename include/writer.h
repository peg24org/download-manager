#ifndef _WRITER_H
#define _WRITER_H

#include <memory>

#include "file_io.h"
#include "download_state_manager.h"

class Writer
{
  public:
  Writer(std::shared_ptr<FileIO> file_io,
      std::shared_ptr<DownloadStateManager> download_state_manager);
  void write(const char* buffer, size_t length, size_t position, size_t index);
  size_t get_file_size() const;
  size_t get_total_written_bytes() const;

  private:
  std::shared_ptr<FileIO> file_io;
  std::shared_ptr<DownloadStateManager> download_state_manager;
};

#endif
