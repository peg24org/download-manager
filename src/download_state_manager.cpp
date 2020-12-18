#include "download_state_manager.h"

#include <bits/stdc++.h>

using namespace std;

DownloadStateManager::DownloadStateManager(shared_ptr<FileIO> file_io)
  : file_io(move(file_io))
{
}

void DownloadStateManager::set_initial_state(const ChunksCollection&
    chunks_collection, size_t file_size)
{
  this->chunks_collection = chunks_collection;
  this->file_size = file_size;
  file_io->create();
  store();
}

void DownloadStateManager::update(size_t index, size_t current_pos)
{
  lock_guard<mutex> guard(chunks_collection_mutex);
  chunks_collection[index].current_pos = current_pos;
  store();
}

void DownloadStateManager::store()
{
  // <index> <strart position> <current position> <end position>
  string download_state = to_string(file_size) + "\n";
  for (auto download_chunk : chunks_collection) {
    // Thread index
    download_state += to_string(download_chunk.first) + " ";
    // Start position
    download_state += to_string(download_chunk.second.start_pos) + " ";
    // Current position
    download_state += to_string(download_chunk.second.current_pos) + " ";
    // End position
    download_state += to_string(download_chunk.second.end_pos) + "\n";
  }
  file_io->write(download_state.c_str(), download_state.length());
}

ChunksCollection DownloadStateManager::get_download_chunks()
{
  retrieve();
  return chunks_collection;
}

ChunksCollection DownloadStateManager::get_chunks_collection() const
{
  lock_guard<mutex> guard(chunks_collection_mutex);
  return chunks_collection;
}

void DownloadStateManager::retrieve()
{
  file_io->open();
  chunks_collection.clear();
  istringstream file_contents(file_io->get_file_contents());
  file_contents >> file_size;

  DownloadChunk chunk;
  size_t index;
  while(file_contents >> index >> chunk.start_pos >> chunk.current_pos >>
      chunk.end_pos)
    chunks_collection[index] = chunk;
}

size_t DownloadStateManager::get_file_size() const
{
  return file_size;
}

size_t DownloadStateManager::get_total_written_bytes() const
{
  size_t total_received_bytes = 0;
  for (auto& chunk : chunks_collection)
    total_received_bytes += (chunk.second.current_pos - chunk.second.start_pos);
  return total_received_bytes;
}

void DownloadStateManager::remove_stat_file()
{
  file_io->remove();
}
