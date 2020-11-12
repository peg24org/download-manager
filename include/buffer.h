#ifndef _BUFFER_H
#define _BUFFER_H

#include <memory>

class Buffer
{
  public:
    // C-tor.
    Buffer(size_t capacity = kDefaultCapacity);
    // Copy C-tor.
    Buffer(const Buffer& src);

    Buffer& operator=(const Buffer& src);
    /**
     * Char* cast operator overloading.
     *
     * @return Address of data buffer.
     */
    operator char*();

    /**
     * Resizes the buffer capacity.
     * Note: This function removes former data and creates new data buffer.
     *
     * @param new_capacity New capacity of buffer
     * @throws std::bad_alloc in case of failure.
     * @return True in case of resizing is successful, otherwise returns False.
     */
    void resize(size_t capacity);
    /**
     * Gets capacity of buffer.
     *
     * @return Capacity of buffer
     */
    size_t capacity() const;

    // Length of contents in buffer.
    size_t length;

    // Default capacity of buffer.
    constexpr static size_t kDefaultCapacity = 40 * 1024;

  private:
    void allocate(size_t capacity);
    size_t buffer_capacity;
    std::unique_ptr<char[]> buffer;
};

#endif
