#include "state_manager.h"
#include <iostream>
#include <sstream>

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
      new_part.second = Chunk(last_chunk.end, last_chunk.end + 1, end);
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
  istringstream file_contents(state_file->get_file_contents());
  file_contents >> download_file_size;

  Chunk chunk;
  ssize_t index;
  while(file_contents >> index >> chunk.start >> chunk.current >> chunk.end) {
    if (index < 0)
      break;

    total_recvd_bytes += (chunk.current - chunk.start);

    parts[index] = chunk;
    if (chunk.current < chunk.end)
      initial_parts.push(make_pair(index, chunk));
  }

  initial_index = index;
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
  // <index> <start position> <current position> <end position>
  string download_state = to_string(download_file_size) + "\n";
  for (auto chunk : parts) {
    // Chunk index
    download_state += to_string(chunk.first) + " ";
    // Start position
    download_state += to_string(chunk.second.start) + " ";
    // Current position
    download_state += to_string(chunk.second.current) + " ";
    // End position
    download_state += to_string(chunk.second.end) + "\n";
  }
  download_state += "-1 0 0 0\n";
  state_file->write(download_state.c_str(), download_state.length());
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

