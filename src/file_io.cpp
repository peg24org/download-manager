#include "file_io.h"

#include <cassert>

using namespace std;

FileIO::FileIO(string file_name) : file_name(file_name), file_pointer(nullptr)
{
}

void FileIO::open()
{
  // TODO: check error
  // Open existing file for reading and writing
  file_pointer = fopen(file_name.c_str(), "rb+");
}

void FileIO::create(size_t file_length)
{
  // TODO: check error
  // Open new file
  file_pointer = fopen(file_name.c_str(), "wb+");

  for (size_t i = 0; i < file_length - 1; i++)
    fputc('\0', file_pointer);

  rewind(file_pointer);
}

void FileIO::write(size_t pos, size_t len, char* buf)
{
  const lock_guard<mutex> lock(io_mutex);

  assert(file_pointer);
  fseek(file_pointer, pos, SEEK_SET);
  fwrite(buf, 1,len, file_pointer);
}
