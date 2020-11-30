#include "buffer.h"

#include <utility>
#include <cstring>
#include <algorithm>

using namespace std;

Buffer::Buffer(size_t capacity) : buffer_length(0)
{
  allocate(capacity);
}

Buffer::Buffer(const Buffer& src) : buffer_length(src.buffer_length)
{
  allocate(src.buffer_capacity);
  memcpy(buffer.get(), src.buffer.get(), buffer_length);
}

Buffer::Buffer(Buffer&& src) noexcept
  : buffer_length(src.buffer_length)
  , buffer_capacity(src.buffer_capacity)
  , buffer(exchange(src.buffer, nullptr))
{
}

Buffer& Buffer::operator=(const Buffer& src)
{
  buffer_length = src.buffer_length;
  if (src.buffer_length > this->buffer_capacity)
    allocate(src.buffer_capacity);
  memcpy(buffer.get(), src.buffer.get(), buffer_length);

  return *this;
}

Buffer& Buffer::operator=(Buffer&& src) noexcept
{
  swap(this->buffer, src.buffer);
  src.buffer = nullptr;
  this->buffer_length = src.buffer_length;
  this->buffer_capacity = src.buffer_capacity;

  return *this;
}

Buffer::operator char*()
{
  return buffer.get();
}

Buffer& Buffer::operator<<(const char* input)
{
  size_t input_len = strlen(input);
  size_t necessary_len = input_len+buffer_length;
  if (necessary_len > buffer_capacity)
    extend(necessary_len * 2);

  memcpy(buffer.get() + buffer_length, input, input_len);
  buffer_length += input_len;

  return *this;
}

Buffer& Buffer::operator<<(const std::string& input)
{
  const size_t input_length = input.length();
  size_t necessary_len = input_length + buffer_length;
  if (necessary_len > buffer_capacity)
    extend(necessary_len * 2);

  memcpy(buffer.get() + buffer_length, input.c_str(), input_length);
  buffer_length += input_length;

  return *this;
}

Buffer& Buffer::operator<<(char input)
{
  size_t necessary_len = buffer_length + 1;
  if (necessary_len > buffer_capacity)
    extend(necessary_len * 2);

  buffer.get()[buffer_length] = input;
  ++buffer_length;

  return *this;
}

Buffer& Buffer::operator<<(int input)
{
  *this << to_string(input);

  return *this;
}

void Buffer::set_capacity(size_t capacity)
{
  this->buffer_capacity = capacity;
  buffer = make_unique<char[]>(capacity);
}

void Buffer::extend(size_t capacity)
{
  buffer_capacity = capacity;
  unique_ptr<char[]> temp_buffer = make_unique<char[]>(buffer_capacity);
  memcpy(temp_buffer.get(), buffer.get(), buffer_length);
  buffer = move(temp_buffer);
}

size_t Buffer::capacity() const noexcept
{
  return buffer_capacity;
}

size_t Buffer::length() const noexcept
{
  return buffer_length;
}

void Buffer::set_length(size_t length) noexcept
{
  buffer_length = length;
}

void Buffer::clear() noexcept
{
  buffer_length = 0;
}

void Buffer::deep_clear() noexcept
{
  clear();
  memset(buffer.get(), '\0', buffer_capacity);
}

void Buffer::allocate(size_t capacity)
{
  buffer = make_unique<char[]>(capacity);
  this->buffer_capacity = capacity;
}
