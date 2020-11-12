#include "buffer.h"

#include <iostream>
#include <cstring>

using namespace std;

Buffer::Buffer(size_t capacity) : length(0)
{
  allocate(capacity);
}

Buffer::Buffer(const Buffer& src) : length(src.length)
{
  allocate(src.buffer_capacity);
  memcpy(buffer.get(), src.buffer.get(), length);
}

Buffer& Buffer::operator=(const Buffer& src)
{
  length = src.length;
  if (src.length > this->buffer_capacity)
    allocate(src.buffer_capacity);
  memcpy(buffer.get(), src.buffer.get(), length);

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

size_t Buffer::capacity() const
{
  return buffer_capacity;
}

void Buffer::allocate(size_t capacity)
{
  buffer = make_unique<char[]>(capacity);
  this->buffer_capacity = capacity;
}
