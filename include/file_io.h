#ifndef _FILE_IO_H
#define _FILE_IO_H

#include <string>
#include <fstream>

#include "buffer.h"

enum class PathType
{
  FILE_T,
  DIRECTORY_T,
  UNKNOWN_T
};

class FileIO {

  public:
    ~FileIO();

    FileIO(std::string path);

    /// Creates new file.
    virtual void create(size_t file_length = 0);

    /// Opens existing file.
    virtual void open();

    /**
     * Writes a buffer in the file.
     *
     * @param buffer Buffer to write in file
     * @param length Length of buffer
     * @param position Position of buffer in file
     */
    virtual void write(const char* buffer, size_t length, size_t position=0);

    /**
     * Writes a buffer in the file.
     *
     * @param buffer Buffer to write in file
     * @param position Position of buffer in file
     */
    virtual void write(const Buffer& buffer, size_t position=0);

    virtual bool check_existence() const;

    virtual PathType check_path_type();

    /**
     * Returns one line of file.
     *
     * @param line_number Line number of file to get
     * @throws std::runtime_error Thrown if 'open' function is not called
     * @return One line of file
     */
    virtual std::string get_file_contents();

    std::fstream& get_file_stream();

    virtual void remove();

  private:
    std::string path;
    std::fstream file_stream;
};

#endif
