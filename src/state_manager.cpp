#include "state_manager.h"

#include <cmath>
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
  if (total_recvd_bytes >= file_size)
    state_file->remove();
}

StateManager::StateManager(const string& file_path, size_t chunk_len)
  : file_size(0)
  , total_recvd_bytes(0)
  , chunk_len(chunk_len)
  , chunks_num(0)
  , chunks_num_max(1)
{
  state_file = make_unique<FileIO>(file_path);
}

bool StateManager::state_file_available() const
{
  bool exist = state_file->check_existence();
  return exist;
}

void StateManager::generate_parts()
{
  if (parts.size() == 0) {
    for (uint16_t i = 0; i < chunks_num; ++i) {
      const size_t start = i * chunk_len;
      const size_t end = i < chunks_num - 1 ? start + chunk_len - 1 : file_size;
      const size_t& current = start;
      Chunk new_chunk(start, current, end);
      parts[i] = new_chunk;
    }
  }
}

size_t StateManager::get_chunks_num() const
{
  return parts.size();
}

uint16_t StateManager::get_chunks_num_max() const
{
  return chunks_num_max;
}

vector<uint16_t> StateManager::get_parts()
{
  vector<uint16_t> result;
  for (const auto& part : parts) {
    if (result.size() < chunks_num)
      result.push_back(part.first);
    else
      break;
  }
  return result;
}

size_t StateManager::get_end_pos(uint16_t index) const
{
  return parts.at(index).end;
}

size_t StateManager::get_start_pos(uint16_t index) const
{
  return parts.at(index).start;
}

size_t StateManager::get_current_pos(uint16_t index) const
{
  return parts.at(index).current;
}

void StateManager::create_new_state(size_t file_size)
{
  if (file_size == 0)
    throw runtime_error("StateManager: File size should not be zero.");

  chunks_num_max = file_size / chunk_len ;
  chunks_num_max = chunks_num_max > 0 ? chunks_num_max : 1;
  parts.clear();
  this->file_size = file_size;
  state_file->create();
}

void StateManager::set_chunks_num(uint16_t chunks_num)
{
  this->chunks_num = (chunks_num>=chunks_num_max) ? chunks_num_max : chunks_num;
  this->chunk_len = file_size / this->chunks_num;
  generate_parts();
}

size_t StateManager::get_file_size() const
{
  return file_size;
}

size_t StateManager::get_total_recvd_bytes() const
{
  return total_recvd_bytes;
}

size_t StateManager::get_chunks_max() const
{
  return chunks_num;
}

void StateManager::retrieve()
{
  if (!state_file_available())
    throw runtime_error("*.stat file not available.");

  state_file->open();
  parts.clear();

  fstream& file_stream = state_file->get_file_stream();
  file_stream.read(reinterpret_cast<char*>(&file_size), sizeof(size_t));
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
      parts[temp_index] = temp_chunk;
  }
}

void StateManager::update(size_t index, size_t recvd_bytes)
{
  parts[index].current += recvd_bytes;
  if (parts[index].current >= parts[index].end) {
    parts[index].finished = true;
    parts[index].busy = false;
  }
  total_recvd_bytes += recvd_bytes;
  store();
}

void StateManager::erase_part(uint32_t index)
{
  parts.erase(index);
}

void StateManager::store()
{
  // <file_size: size_t> <number_of_blocks: size_t>
  // <blocks: 4 x size_t x number_of_blocks>
  size_t buff_total_size = 2 * sizeof(size_t);
  size_t buff_pos = 0;
  buff_total_size += parts.size() * 4 * sizeof(size_t);
  char* buffer = new char[buff_total_size];
  memcpy(buffer, reinterpret_cast<char*>(&file_size), sizeof(size_t));
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
  vector<uint16_t> finished_parts;

  for (const auto& part : parts)
    if (part.second.finished)
      finished_parts.push_back(part.first);

  for (uint16_t i : finished_parts)
    parts.erase(i);
}
