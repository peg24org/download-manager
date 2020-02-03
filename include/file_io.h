#ifndef FILE_IO_H
#define FILE_IO_H

#include <string>
#include <fstream>

class FileIO {
  public:
    FileIO(std::string file_name);
    /// Creates new file.
    void create(size_t file_length = 0);
    /// Opens existing file.
    void open();
    /**
     * Writes a buffer in file.
     *
     * @param position Position of buffer in file
     * @param length Length of buffer
     * @param buffer Buffer to write in file
     */
    void write(const char* buffer, size_t position, size_t length);
    /**
     * Writes a buffer in file, previous content is deleted and
     * replaced by the new one.
     *
     * @param length Length of buffer
     * @param buffer Buffer to write in file
     */
    void write(const char* buffer, size_t length);
    /**
     * Checks the file exists or not.
     * @return The file existence checking result, 'true' if file exists.
     */
    bool check_existence();

    /**
     * Returns one line of file.
     * @param line_number Line number of file to get
     * @return One line of file
     */
    std::string get_file_contents();
    void remove();

  private:
    std::string file_name;
    std::fstream file_stream;
};

#endif
