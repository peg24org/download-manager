#include "writer.h"

#include <iostream>
#include <memory>
#include <gtest/gtest.h>

#include "file_io.h"
#include "test_utils.h"
#include "download_state_manager.h"

using namespace std;

class WriterTest : public ::testing::Test
{
  public:
  virtual void SetUp() {
    state_file_io = make_unique<StatFileIOMock>();
    download_state_manager =
      make_shared<DownloadStateManager>(move(state_file_io));
    main_file_io = make_shared<FileIOMock>();

    main_file_io->create(kMainMockFileLength);
    writer = make_shared<Writer>(main_file_io, download_state_manager);

    writer->write(kString_0, strlen(kString_0), kPosition_0, kIndex_0);
    writer->write(kString_1, strlen(kString_1), kPosition_1, kIndex_1);
    chunks_collection = download_state_manager->get_download_chunks();
  }

  protected:
  static constexpr size_t kMainMockFileLength = 100 * 1024;
  static constexpr char kString_0[] = "Sample streing 1.";
  static constexpr char kString_1[] = "Sample_streing_2.";
  static constexpr size_t kPosition_0 = 12;
  // Preventing overlap
  static constexpr size_t kPosition_1 = 20 + kPosition_0 + strlen(kString_1);
  static constexpr size_t kIndex_0 = 0;
  static constexpr size_t kIndex_1 = 1;
  static constexpr size_t kNumberOfIndices = 2;
  ChunksCollection chunks_collection;

  shared_ptr<Writer> writer;
  shared_ptr<FileIO> main_file_io;
  unique_ptr<FileIO> state_file_io;
  shared_ptr<DownloadStateManager> download_state_manager;
};

TEST_F(WriterTest, retrieved_pos_should_be_same_as_stored_pos_plus_strlen)
{
  constexpr size_t kPos_0 = kPosition_0 + strlen(kString_0);
  constexpr size_t kPos_1 = kPosition_1 + strlen(kString_1);

  EXPECT_EQ(kPos_0, chunks_collection[kIndex_0].current_pos);
  EXPECT_EQ(kPos_1, chunks_collection[kIndex_1].current_pos);
}

TEST_F(WriterTest,
    retrieved_number_of_indices_should_be_same_stored_chunks_using_writer)
{
  EXPECT_EQ(kNumberOfIndices, chunks_collection.size());
}

TEST_F(WriterTest, retrieved_string_should_be_same_as_stored_using_writer)
{
  char* contents = dynamic_cast<FileIOMock*>(main_file_io.get())->get_file_buffer();
  char* sub_str_0 = contents + kPosition_0;
  char* sub_str_1 = contents + kPosition_1;
  EXPECT_EQ(0, strncmp(sub_str_0, sub_str_0, strlen(kString_0)));
  EXPECT_EQ(0, strncmp(sub_str_1, sub_str_1, strlen(kString_1)));
}


class WriterTestBufferInterface : public WriterTest
{
  protected:
    static constexpr size_t kBuffer0Length = 3000;
    static constexpr size_t kBuffer1Length = 6000;
    Buffer test_buffer_0;
    Buffer test_buffer_1;
    size_t kPosition_1;

    virtual void SetUp() {
      state_file_io = make_unique<StatFileIOMock>();
      download_state_manager =
        make_shared<DownloadStateManager>(move(state_file_io));

      main_file_io = make_shared<FileIOMock>();
      main_file_io->create(kMainMockFileLength);

      writer = make_shared<Writer>(main_file_io, download_state_manager);

      unique_ptr<char[]> buffer_0 = get_random_buffer(kBuffer0Length);
      unique_ptr<char[]> buffer_1 = get_random_buffer(kBuffer1Length);
      memcpy(test_buffer_0, buffer_0.get(), kBuffer0Length);
      memcpy(test_buffer_1, buffer_1.get(), kBuffer1Length);

      test_buffer_0.set_length(kBuffer0Length);
      test_buffer_1.set_length(kBuffer1Length);

      kPosition_1 = kPosition_0 + test_buffer_0.length();
      writer->write(test_buffer_0, kPosition_0, kIndex_0);
      writer->write(test_buffer_1, kPosition_1, kIndex_1);
      chunks_collection = download_state_manager->get_download_chunks();
    }
};

TEST_F(WriterTestBufferInterface,
       retrieved_pos_should_be_same_as_stored_pos_plus_strlen_using_writer)
{
  size_t kPos_0 = kPosition_0 + test_buffer_0.length();
  size_t kPos_1 = kPosition_1 + test_buffer_1.length();

  EXPECT_EQ(kPos_0, chunks_collection[kIndex_0].current_pos);
  EXPECT_EQ(kPos_1, chunks_collection[kIndex_1].current_pos);
}

TEST_F(WriterTestBufferInterface,
       retrieved_number_of_indices_should_be_same_stored_chunks_using_writer)
{
  EXPECT_EQ(kNumberOfIndices, chunks_collection.size());
}

TEST_F(WriterTestBufferInterface,
       retrieved_string_should_be_same_as_stored_using_writer)
{
  char* buffer =
    (dynamic_cast<FileIOMock*>(main_file_io.get()))->get_file_buffer();

  char* buffer_0 = buffer + kPosition_0;
  char* buffer_1 = buffer + kPosition_1;

  EXPECT_EQ(0, strncmp(test_buffer_0, buffer_0, test_buffer_0.length()));
  EXPECT_EQ(0, strncmp(test_buffer_1, buffer_1, test_buffer_1.length()));
}
