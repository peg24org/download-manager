#include "file_io.h"

#include <sys/stat.h>

#include <cstdio>
#include <iostream>
#include <streambuf>

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

  for (size_t i = 1; i < file_length; i++)
    file_stream << '\0';
  file_stream.clear();
  file_stream.seekg(0);
}

void FileIO::write(const char* buf, size_t pos, size_t len)
{
  file_stream.seekp(pos);
  file_stream.write(buf, len);
}

void FileIO::write(const char* buf, size_t len)
{
  file_stream.seekp(0);
  file_stream.write(buf, len);
}

bool FileIO::check_existence()
{
  struct stat stat_buf;
  return stat(file_name.c_str(), &stat_buf) == 0;
}

string FileIO::get_file_contents()
{
  file_stream.seekp(0);
  return string((istreambuf_iterator<char>(file_stream)),
      istreambuf_iterator<char>());
}

void FileIO::remove()
{
  file_stream.close();
  ::remove(file_name.c_str());
}
