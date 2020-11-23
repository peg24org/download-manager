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

void Buffer::resize(size_t capacity)
{
  this->buffer_capacity = capacity;
  buffer = make_unique<char[]>(capacity);
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
