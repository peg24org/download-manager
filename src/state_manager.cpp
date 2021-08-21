#include "state_manager.h"
#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;

Chunk::Chunk() : start(0), current(0), end(0), finished(false)
{
}

Chunk::Chunk(size_t start, size_t current, size_t end)
  : start(start)
  , current(current)
  , end(end)
  , finished(false)
{
}

bool Chunk::operator==(const Chunk& rhs) const
{
  return start == rhs.start &&
    current == rhs.current &&
    end == rhs.end;
}

StateManager::~StateManager()
{
  if (total_recvd_bytes >= download_file_size)
    state_file->remove();
}

StateManager::StateManager(const string& file_path)
  : download_file_size(0)
  , total_recvd_bytes(0)
  , chunk_size(kMinChunkSize)
  , inited(false)
{
  state_file = make_unique<FileIO>(file_path);
}

bool StateManager::state_file_available() const
{
  bool exist = state_file->check_existence();
  if (!exist)
    state_file->create();

  return exist;
}

bool StateManager::part_available() const
{
  bool new_part = true;
  if (parts.size() > 0) {
    Chunk last_chunk = parts.rbegin()->second;
    size_t last_chunk_size = last_chunk.end - last_chunk.current;
    new_part = (last_chunk.end != download_file_size) || 
               (last_chunk_size > chunk_size);
  }

  return (new_part && download_file_size > 0) ||
          initial_parts.size() > 0;
}

pair<size_t, Chunk> StateManager::get_part()
{
  if (!part_available())
    throw runtime_error("new part not available.");

  pair<size_t, Chunk> new_part;

  if (initial_parts.size() > 0) {
    pair<size_t, Chunk> pop_part = initial_parts.front();
    initial_parts.pop();
    if (pop_part.second.end == download_file_size) {
      size_t new_chunk_size = pop_part.second.end - pop_part.second.current;
      if (new_chunk_size > chunk_size)
        pop_part.second.end = pop_part.second.current + chunk_size;
    }
    new_part = pop_part;
  }
  else {
    if (parts.size() == 0) {
      new_part.first = 0;
      new_part.second = Chunk(0, 0, chunk_size);
    }
    else {
      map<size_t, Chunk>::reverse_iterator last_part = parts.rbegin();
      Chunk last_chunk = last_part->second;
      new_part.first = last_part->first + 1;
      size_t end = last_chunk.end + chunk_size;
      if (end > download_file_size) // Check for last chunk
        end = download_file_size;
      new_part.second = Chunk(last_chunk.end, last_chunk.end, end);
    }
  }
  parts[new_part.first] = new_part.second;

  return new_part;
}

void StateManager::create_new_state(size_t download_file_size)
{
  if (download_file_size == 0)
    throw runtime_error("StateManager: File size should not be zero.");

  parts.clear();
  this->download_file_size = download_file_size;
  inited = true;
}

void StateManager::set_chunk_size(size_t chunk_size)
{
  if (chunk_size > kMinChunkSize)
    this->chunk_size = chunk_size;
}

size_t StateManager::get_chunk_size() const
{
  return chunk_size;
}

size_t StateManager::get_file_size() const
{
  return download_file_size;
}

size_t StateManager::get_total_recvd_bytes() const
{
  return total_recvd_bytes;
}

queue<pair<size_t, Chunk>> StateManager::get_initial_parts() const
{
  return initial_parts;
}

void StateManager::retrieve()
{
  if (!state_file_available())
    throw runtime_error("*.stat file not available.");

  state_file->open();
  parts.clear();

  fstream& file_stream = state_file->get_file_stream();
  file_stream.read(reinterpret_cast<char*>(&download_file_size), sizeof(size_t));
  size_t blocks_size;
  file_stream.read(reinterpret_cast<char*>(&blocks_size), sizeof(size_t));
  size_t temp_index = 0;
  Chunk temp_chunk;

  for (size_t i = 0; i < blocks_size; ++i) {
    file_stream.read(reinterpret_cast<char*>(&temp_index), sizeof(size_t));
    file_stream.read(reinterpret_cast<char*>(&temp_chunk.start), sizeof(size_t));
    file_stream.read(reinterpret_cast<char*>(&temp_chunk.current), sizeof(size_t));
    file_stream.read(reinterpret_cast<char*>(&temp_chunk.end), sizeof(size_t));

    total_recvd_bytes += (temp_chunk.current - temp_chunk.start);
    parts[temp_index] = temp_chunk;

    if (temp_chunk.current < temp_chunk.end)
      initial_parts.push(make_pair(temp_index, temp_chunk));
  }
  initial_index = temp_index;
  inited = true;
}

void StateManager::update(size_t index, size_t recvd_bytes)
{
  parts[index].current += recvd_bytes;
  if (parts[index].current >= parts[index].end) {
    parts[index].finished = true;
    parts[index].busy = false;
  }

  total_recvd_bytes += recvd_bytes;

  remove_finished_parts();

  store();
}

void StateManager::store()
{
  // <file_size: size_t> <number_of_blocks: size_t>
  // <blocks: 4 x size_t x number_of_blocks>
  size_t buff_total_size = 2 * sizeof(size_t);
  size_t buff_pos = 0;
  buff_total_size += parts.size() * 4 * sizeof(size_t);
  char* buffer = new char[buff_total_size];
  memcpy(buffer, reinterpret_cast<char*>(&download_file_size), sizeof(size_t));
  buff_pos = sizeof(size_t);
  size_t records_size = parts.size();
  memcpy(buffer + buff_pos, reinterpret_cast<char*>(&records_size), sizeof(size_t));
  buff_pos += sizeof(size_t);
  for (auto chunk : parts) {
    // Chunk index
    memcpy(buffer + buff_pos, reinterpret_cast<const char*>(&chunk.first), sizeof(size_t));
    buff_pos += sizeof(size_t);
    // Start position
    memcpy(buffer + buff_pos, reinterpret_cast<const char*>(&chunk.second.start), sizeof(size_t));
    buff_pos += sizeof(size_t);
    // Current position
    memcpy(buffer + buff_pos, reinterpret_cast<const char*>(&chunk.second.current), sizeof(size_t));
    buff_pos += sizeof(size_t);
    // End position
    memcpy(buffer + buff_pos, reinterpret_cast<char*>(&chunk.second.end), sizeof(size_t));
    buff_pos += sizeof(size_t);
  }

  state_file->write(buffer, buff_total_size);

  delete[] buffer;
}

void StateManager::remove_finished_parts()
{
  vector<size_t> finished_parts;

  for (const auto& part : parts)
    if (part.second.finished) {
      finished_parts.push_back(part.first);
    }

  if (finished_parts.size() >= 2) {
    if (parts[finished_parts[0]].end == parts[finished_parts[1]].start) {
      parts[finished_parts[0]].end = parts[finished_parts[1]].end;
      parts[finished_parts[0]].current = parts[finished_parts[1]].current;
      parts.erase(finished_parts[1]);
    }
  }
}

