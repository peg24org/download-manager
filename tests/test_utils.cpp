#include "test_utils.h"

#include <chrono>
#include <random>
#include <cstring>
#include <algorithm>

using namespace std;

void FileIOMock::create(size_t file_length)
{
  file_buffer = make_unique<char[]>(file_length);
  file_opened = true;
}

void FileIOMock::open()
{
  file_opened = true;
}

void FileIOMock::write(const char* buffer, size_t length, size_t position)
{
  memcpy(file_buffer.get() + position, buffer, length);
}

char* FileIOMock::get_file_buffer()
{
  return file_buffer.get();
}

void StatFileIOMock::write(const char* buffer, size_t length, size_t position)
{
  file_contents.replace(position, length, buffer);
}

string StatFileIOMock::get_file_contents()
{
  if (!file_opened)
    throw std::runtime_error("FileIO is not open.");
  return file_contents;
}

std::unique_ptr<char[]> get_random_buffer(size_t length,
                                          char start_char, char end_char)
{
  std::unique_ptr<char[]> buffer = std::make_unique<char[]>(length);
  unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
  std::default_random_engine random_engine(seed);
  std::uniform_int_distribution<char> distribution(start_char, end_char);

  for (size_t i = 0; i < length; ++i)
    buffer.get()[i] = distribution(random_engine);

  return buffer;
}

string get_random_string(size_t length)
{
  // Get printable characters
  unique_ptr<char[]> random_buffer = get_random_buffer(length, 0x21, 0x7e);
  return string(random_buffer.get(), length);
}
