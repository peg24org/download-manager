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

    // Move C-tor
    Buffer(Buffer&& src) noexcept;

    // Copy assignment
    Buffer& operator=(const Buffer& src);

    // Move assignment
    Buffer& operator=(Buffer&& src) noexcept;

    /**
     * Char* cast operator overloading.
     *
     * @return Address of data buffer.
     */
    operator char*();

    // Caution: always gets null terminated c-string.
    Buffer& operator<<(const char* input);

    Buffer& operator<<(const std::string& input);

    Buffer& operator<<(char input);

    Buffer& operator<<(int input);

    /**
     * Resizes the buffer capacity.
     * Note: This function removes former data and creates new data buffer.
     *
     * @param capacity New capacity of buffer
     * @throws std::bad_alloc in case of failure.
     */
    void set_capacity(size_t capacity);

    /**
     * Extends capacity of buffer.
     * Note: This function won't change the contents of buffer.
     *
     * @param capacity New capacity of buffer
     * @throws std::bad_alloc in case of failure.
     */
    void extend(size_t capacity);
    /**
     * Gets capacity of buffer.
     *
     * @return Capacity of buffer
     */
    size_t capacity() const noexcept;

    // Gets length of buffer.
    size_t length() const noexcept;

    void set_length(size_t length) noexcept;

    // Clears buffer, only sets length to 0 and doesn't change contents.
    void clear() noexcept;

    // Clears buffer, set contents to null and length to 0.
    void deep_clear() noexcept;

    Buffer& seek(size_t position);

    // Default capacity of buffer.
    constexpr static size_t kDefaultCapacity = 40 * 1024;

  private:
    void allocate(size_t capacity);
    // Length of contents in buffer.
    size_t buffer_length;
    size_t buffer_capacity;
    std::unique_ptr<char[]> buffer;
    size_t index_position;
};

#endif
