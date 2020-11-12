#include "test_utils.h"

#include <chrono>
#include <random>

using namespace std;

void FileIOMock::create(size_t file_length)
{
  file_contents = std::string(file_length, '\0');
  file_opened = true;
}

void FileIOMock::open()
{
  file_opened = true;
}

void FileIOMock::write(const char* buffer, size_t length, size_t position)
{
  file_contents.replace(position, length, buffer);
}

string FileIOMock::get_file_contents()
{
  if (!file_opened)
    throw std::runtime_error("FileIO is not open.");
  return file_contents;
}

void FileIOMock::remove()
{
}

std::unique_ptr<char[]> get_random_buffer(size_t length)
{
  std::unique_ptr<char[]> buffer = std::make_unique<char[]>(length);
  unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
  std::default_random_engine random_engine(seed);
  std::uniform_int_distribution<char> distribution(0, 255);

  for (size_t i = 0; i < length; ++i)
    buffer.get()[i] = distribution(random_engine);

  return buffer;
}

bool buffer_cmp(char* buffer_1, char* buffer_2, size_t len)
{
  for (size_t i = 0; i < len; ++i)
    if (buffer_1[i] != buffer_2[i])
      return false;
  return true;
}
