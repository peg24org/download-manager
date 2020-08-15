#ifndef FILE_IO_H
#define FILE_IO_H

#include <string>
#include <fstream>

class FileIO {

  public:
    ~FileIO();

    FileIO(std::string file_name);

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

    virtual bool check_existence();

    /**
     * Returns one line of file.
     * @param line_number Line number of file to get
     * @throws std::runtime_error Thrown if 'open' function is not called
     * @return One line of file
     */
    virtual std::string get_file_contents();

    virtual void remove();

  private:
    std::string file_name;
    std::fstream file_stream;
};

#endif
