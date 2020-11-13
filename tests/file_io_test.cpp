#include <memory>
#include <string>
#include <exception>
#include <gtest/gtest.h>

#include "file_io.h"
#include "test_utils.h"

using namespace std;

class FileIOTest : public ::testing::Test
{
  public:
  FileIOTest(size_t file_length=0)
    : writer(TEST_FILE_NAME)
    , reader(TEST_FILE_NAME) {}

  virtual void SetUp() override {
    writer.create();

    const string kMkdirCommand = "mkdir " + string(TEST_DIR_PATH);
    system(kMkdirCommand.c_str());
  }

  virtual void TearDown() override {
    writer.remove();

    const string kRemDirCommand = "rm -r " + string(TEST_DIR_PATH);
    system(kRemDirCommand.c_str());
  }

  protected:
    static constexpr char TEST_FILE_NAME[] = "TEST_FILE";
    static constexpr char TEST_DIR_PATH[] = "TEST_DIR";
    FileIO writer;
    FileIO reader;
};

TEST_F(FileIOTest, file_should_be_exist_after_calling_create_function)
{
  EXPECT_TRUE(reader.check_existence());
}

TEST_F(FileIOTest, file_io_should_throw_exception_if_is_not_open)
{
  EXPECT_THROW(reader.get_file_contents(), std::runtime_error);
}

TEST_F(FileIOTest, writer_file_length_should_be_same_reader_file_length)
{
  static constexpr size_t FILE_LENGTH = 5000;
  // Remove existing file
  writer.remove();
  // Create file using custom length
  writer.create(FILE_LENGTH);

  reader.open();
  size_t reader_file_length = reader.get_file_contents().length();
  EXPECT_EQ(FILE_LENGTH, reader_file_length);
}

TEST_F(FileIOTest, file_should_not_be_exist_after_calling_remove_function)
{
  writer.remove();
  EXPECT_FALSE(reader.check_existence());
}

TEST_F(FileIOTest, retrieved_file_contents_should_be_same_as_written_contents_0)
{
  static constexpr size_t SIZE_OF_SAMPLE_STRING = 3000;
  const string SAMPLE_STRING(SIZE_OF_SAMPLE_STRING, 'x');

  writer.write(SAMPLE_STRING.c_str(), SAMPLE_STRING.length());

  reader.open();
  string read_string = reader.get_file_contents();
  EXPECT_EQ(SIZE_OF_SAMPLE_STRING, read_string.length());
  EXPECT_EQ(SAMPLE_STRING, read_string);
}

TEST_F(FileIOTest, retrieved_file_contents_should_be_same_as_written_contents_1)
{
  static constexpr size_t kSizeOfTestBuffer = 3000;
  unique_ptr<char[]> random_buffer = get_random_buffer(kSizeOfTestBuffer);
  Buffer test_buffer;
  test_buffer.length = kSizeOfTestBuffer;
  memcpy(test_buffer, random_buffer.get(), kSizeOfTestBuffer);

  writer.write(test_buffer);

  reader.open();
  string read_string = reader.get_file_contents();
  EXPECT_EQ(kSizeOfTestBuffer, read_string.length());
  EXPECT_EQ(0, strncmp(test_buffer, read_string.c_str(), kSizeOfTestBuffer));
}

TEST_F(FileIOTest, written_bytes_in_some_position_should_be_same_as_read_bytes_0)
{
  static constexpr char SAMPLE_SUBSTRING[] = "sample sub-string...";
  static constexpr size_t SIZE_OF_SAMPLE_STRING = 1000;
  const string SAMPLE_STRING(SIZE_OF_SAMPLE_STRING, 'x');

  static constexpr size_t WRITE_POSITION = 100;
  writer.write(SAMPLE_STRING.c_str(), SAMPLE_STRING.length());
  writer.write(SAMPLE_SUBSTRING, strlen(SAMPLE_SUBSTRING),WRITE_POSITION);

  reader.open();
  string read_contents = reader.get_file_contents();
  string read_sample_substr =
    read_contents.substr(WRITE_POSITION, strlen(SAMPLE_SUBSTRING));
  EXPECT_EQ(SAMPLE_SUBSTRING, read_sample_substr);
}

TEST_F(FileIOTest, written_bytes_in_some_position_should_be_same_as_read_bytes_1)
{
  static constexpr size_t kSizeOfTestBuffer = 1000;
  static constexpr size_t kWritePosition = 100;
  unique_ptr<char[]> random_buffer = get_random_buffer(kSizeOfTestBuffer);

  Buffer test_buffer;
  memcpy(test_buffer, random_buffer.get(), kSizeOfTestBuffer);
  test_buffer.length = kWritePosition;
  writer.write(test_buffer);
  writer.write(test_buffer, kWritePosition);

  reader.open();
  string read_contents = reader.get_file_contents();
  string read_sample_substr =
    read_contents.substr(kWritePosition, test_buffer.length);
  const char* read_sample = read_sample_substr.c_str();
  EXPECT_EQ(0, strncmp(test_buffer, read_sample, test_buffer.length));
}

TEST_F(FileIOTest, file_path_should_identifed_as_file_t)
{
  EXPECT_EQ(PathType::FILE_T, reader.check_path_type());
}

TEST_F(FileIOTest, dir_path_should_identified_as_dir_t)
{
  FileIO file_io(TEST_DIR_PATH);
  EXPECT_EQ(PathType::DIRECTORY_T, file_io.check_path_type());
}
