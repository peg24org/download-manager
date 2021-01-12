#include "writer.h"

using namespace std;

Writer::Writer(shared_ptr<FileIO> file_io)
  : file_io(file_io)
{
}

void Writer::write(const Buffer& buffer, size_t position)
{
  file_io->write(const_cast<Buffer&>(buffer), buffer.length(), position);
}

void Writer::write(const char* buffer, size_t length, size_t position)
{
  file_io->write(buffer, length, position);
}

size_t Writer::get_file_size() const
{
  return 0;
}

size_t Writer::get_total_written_bytes() const
{
  return 0;
}
