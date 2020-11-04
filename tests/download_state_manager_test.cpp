#include <gtest/gtest.h>
#include <memory>

#include "file_io.h"
#include "test_utils.h"
#include "download_state_manager.h"

using namespace std;

class DownloadStateManagerTest : public ::testing::Test
{
  public:
  DownloadStateManagerTest() : file_io(make_unique<FileIOMock>()) {}
  virtual void SetUp() {
    srand(time(nullptr));
    for (size_t i = 0; i < NUMBER_OF_CHUNKS; ++i)
      chunks_collection[i] = {
        .start_pos = static_cast<size_t>(rand()),
        .current_pos = static_cast<size_t>(rand()),
        .end_pos = static_cast<size_t>(rand())
      };

    download_state_manager = make_unique<DownloadStateManager>(move(file_io));
    download_state_manager->set_initial_state(chunks_collection,
        FAKE_FILE_SIZE);
  }

  protected:
  unique_ptr<FileIOMock> file_io;
  ChunksCollection chunks_collection; 
  unique_ptr<DownloadStateManager> download_state_manager;

  private:
  constexpr static int FAKE_FILE_SIZE = 10;
  constexpr static int NUMBER_OF_CHUNKS = 10;

};

TEST_F(DownloadStateManagerTest, should_return_same_chunks_collection)
{
  ChunksCollection returned_chunks_collection =
    download_state_manager->get_download_chunks();
  EXPECT_EQ(returned_chunks_collection, chunks_collection);
}

TEST_F(DownloadStateManagerTest, should_change_the_value_of_updated_chunk)
{
  constexpr static size_t UPDATE_INDEX_1 = 0;
  constexpr static size_t UPDATE_INDEX_2 = 1;
  constexpr static size_t POSITION_VALUE_1 = 10001;
  constexpr static size_t POSITION_VALUE_2 = 10002;
  
  download_state_manager->update(UPDATE_INDEX_1, POSITION_VALUE_1);
  download_state_manager->update(UPDATE_INDEX_2, POSITION_VALUE_2);

  ChunksCollection chunks = download_state_manager->get_download_chunks();
  EXPECT_EQ(POSITION_VALUE_1, chunks[UPDATE_INDEX_1].current_pos);
  EXPECT_EQ(POSITION_VALUE_2, chunks[UPDATE_INDEX_2].current_pos);
}
