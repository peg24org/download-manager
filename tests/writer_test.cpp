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
    state_file_io = make_unique<FileIOMock>();
    download_state_manager =
      make_shared<DownloadStateManager>(move(state_file_io));
    main_file_io = make_shared<FileIOMock>();

    main_file_io->create(MAIN_MOCK_FILE_LENGTH);
    writer =
      make_shared<Writer>(main_file_io, download_state_manager);

    prepare_tests();
  }

  protected:
  static constexpr char STRING_0[] = "Sample streing 1.";
  static constexpr char STRING_1[] = "Sample_streing_2.";
  static constexpr size_t POSITION_0 = 12;
  // Preventing overlap
  static constexpr size_t POSITION_1 = 20 + POSITION_0 + strlen(STRING_1);
  static constexpr size_t INDEX_0 = 0;
  static constexpr size_t INDEX_1 = 1;
  static constexpr size_t NUMBER_OF_INDICES = 2;
  ChunksCollection chunks_collection;

  shared_ptr<Writer> writer;
  shared_ptr<FileIO> main_file_io;
  unique_ptr<FileIO> state_file_io;
  shared_ptr<DownloadStateManager> download_state_manager;

  void prepare_tests()
  {
    writer->write(STRING_0, strlen(STRING_0), POSITION_0, INDEX_0);
    writer->write(STRING_1, strlen(STRING_1), POSITION_1, INDEX_1);
    chunks_collection = download_state_manager->get_download_chunks();
  }

  private:
  static constexpr size_t MAIN_MOCK_FILE_LENGTH = 5000;
};

TEST_F(WriterTest,
    retrieved_positon_should_be_same_as_stored_position_using_writer)
{
  EXPECT_EQ(POSITION_0, chunks_collection[INDEX_0].current_pos);
  EXPECT_EQ(POSITION_1, chunks_collection[INDEX_1].current_pos);
}

TEST_F(WriterTest,
    retrieved_number_of_indices_should_be_same_stored_chunks_using_writer)
{
  EXPECT_EQ(NUMBER_OF_INDICES, chunks_collection.size());
}

TEST_F(WriterTest, retrieved_string_should_be_same_as_stored_using_writer)
{
  string sub_string_0 = main_file_io->get_file_contents().substr(POSITION_0,
      strlen(STRING_0));
  string sub_string_1 = main_file_io->get_file_contents().substr(POSITION_1,
      strlen(STRING_1));

  EXPECT_EQ(STRING_0, sub_string_0);
  EXPECT_EQ(STRING_1, sub_string_1);
}
