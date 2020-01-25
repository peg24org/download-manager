#include "file_io.h"

#include <sys/stat.h>

using namespace std;

FileIO::FileIO(string file_name) : file_name(file_name)
{
}

void FileIO::open()
{
  // TODO: check error
  // Open existing file for reading and writing
  file_stream.open(file_name);
}

void FileIO::create(size_t file_length)
{
  // TODO: check error
  // Open new file
  file_stream.open(file_name, fstream::in | fstream::out | fstream::trunc);

  for (size_t i = 0; i < file_length - 1; i++)
    file_stream << '\0';
  file_stream.clear();
  file_stream.seekg(0);
}

void FileIO::write(size_t pos, size_t len, char* buf)
{
  const lock_guard<mutex> lock(io_mutex);

  file_stream.seekp(pos);
  file_stream.write(buf, len);
}

bool FileIO::check_existence()
{
  struct stat stat_buf;
  return stat(file_name.c_str(), &stat_buf) == 0;
}
