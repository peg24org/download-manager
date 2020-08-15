#include "file_io.h"

#include <sys/stat.h>

#include <cstdio>
#include <exception>
#include <streambuf>

using namespace std;

FileIO::~FileIO()
{
  file_stream.close();
}

FileIO::FileIO(string file_name) : file_name(file_name)
{
}

void FileIO::open()
{
  // Open existing file for reading and writing
  if (!check_existence())
    create(0);
  else
    file_stream.open(file_name, fstream::in | fstream::out);
}

void FileIO::create(size_t file_length)
{
  // TODO: check error
  // Open new file
  file_stream.open(file_name, fstream::in | fstream::out | fstream::trunc);

  for (size_t i = 1; i <= file_length; i++)
    file_stream << '\0';
  file_stream.clear();
  file_stream.seekg(0);
}

void FileIO::write(const char* buffer, size_t length, size_t position)
{
  file_stream.seekp(position);
  file_stream.write(buffer, length);
  file_stream.flush();
}

bool FileIO::check_existence()
{
  struct stat stat_buf;
  return stat(file_name.c_str(), &stat_buf) == 0;
}

string FileIO::get_file_contents()
{
  if (!file_stream.is_open())
    throw runtime_error("FileIO is not open.");

  file_stream.seekp(0);
  return string((istreambuf_iterator<char>(file_stream)),
      istreambuf_iterator<char>());
}

void FileIO::remove()
{
  file_stream.close();
  ::remove(file_name.c_str());
}
