#include "writer.h"

using namespace std;

Writer::Writer(shared_ptr<FileIO> file_io,
    shared_ptr<DownloadStateManager> download_state_manager)
  : file_io(file_io)
  , download_state_manager(download_state_manager)
{
}

void Writer::write(const char* buffer, size_t length, size_t position,
    size_t index)
{
  file_io->write(buffer, length, position);
  download_state_manager->update(index, position + length);
}

size_t Writer::get_file_size() const
{
	return download_state_manager->get_file_size();
}

size_t Writer::get_total_written_bytes() const
{
  return download_state_manager->get_total_written_bytes();
}
