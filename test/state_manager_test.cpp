#include <tuple>
#include <cmath>
#include <memory>
#include <limits>
#include <algorithm>
#include <exception>

#include <gtest/gtest.h>

#include "units.h"
#include "file_io.h"
#include "test_utils.h"
#include "state_manager.h"

using namespace std;

class StateManagerChunkingTest : public ::testing::Test
{
  protected:
    void SetUp(uint16_t number_of_chunks, size_t file_size = kFileSize)
    {
      state_manager = make_unique<StateManager>("null_file");
      state_manager->create_new_state(file_size);
      state_manager->set_chunks_num(number_of_chunks);
    }
    void TearDown()
    {
      system("rm null_file");
    }

    static constexpr size_t kFileSize = 4 * pow(2, 30);
    unique_ptr<StateManager> state_manager;
};

TEST_F(StateManagerChunkingTest, chunks_number_should_be_non_zero)
{
  static constexpr uint16_t kChunksMax = 21;
  SetUp(kChunksMax);
  EXPECT_EQ(kChunksMax, state_manager->get_chunks_num());
}

TEST_F(StateManagerChunkingTest, chunks_number_should_be_less_than_max)
{
  static constexpr uint16_t kChunksMax = 21;
  SetUp(kChunksMax);
  EXPECT_LE(state_manager->get_chunks_num(), state_manager->get_chunks_num());
}

TEST_F(StateManagerChunkingTest, chunks_number_should_be_equal_to_max)
{
  static constexpr uint16_t kChunksMax = numeric_limits<uint16_t>::max();
  SetUp(kChunksMax);
  EXPECT_EQ(state_manager->get_chunks_num_max(), state_manager->get_chunks_num());
}

TEST_F(StateManagerChunkingTest, chunk_len_should_be_non_negative)
{
  static constexpr uint16_t kChunksMax = 21;
  SetUp(kChunksMax);
  vector<uint16_t> parts_list = state_manager->get_parts();
  for (uint16_t part_index : parts_list)
    EXPECT_NE(state_manager->get_end_pos(part_index) -
              state_manager->get_start_pos(part_index), 0);
}

TEST_F(StateManagerChunkingTest, chunk_start_pos_should_be_less_than_end_pos)
{
  static constexpr uint16_t kChunksMax = 21;
  SetUp(kChunksMax);
  vector<uint16_t> parts_list = state_manager->get_parts();
  for (uint16_t part_index : parts_list)
    EXPECT_LE(state_manager->get_start_pos(part_index),
              state_manager->get_start_pos(part_index));
}

TEST_F(StateManagerChunkingTest, chunk_end_pos_should_be_equal_file_size)
{
  static constexpr uint16_t kChunksMax = 1;
  SetUp(kChunksMax);
  vector<uint16_t> parts_list = state_manager->get_parts();
  sort(parts_list.begin(), parts_list.end());
  size_t last_index = parts_list.at(parts_list.size() - 1);
  EXPECT_EQ(kFileSize, state_manager->get_end_pos(last_index));
}

TEST_F(StateManagerChunkingTest, last_chunk_end_pos_should_be_equal_file_size)
{
  static constexpr uint16_t kChunksMax = 4005;
  SetUp(kChunksMax);
  vector<uint16_t> parts_list = state_manager->get_parts();
  sort(parts_list.begin(), parts_list.end());
  size_t last_index = parts_list.at(parts_list.size() - 1);
  EXPECT_EQ(kFileSize, state_manager->get_end_pos(last_index));
}

class StateManagerStoringTest : public ::testing::Test
{
  protected:
    void SetUp(uint16_t number_of_chunks, map<uint16_t, size_t> samples)
    {
      StateManager state_manager = StateManager(kFilePath);
      state_manager.create_new_state(kFileSize);
      state_manager.set_chunks_num(number_of_chunks);
      for (const auto sample : samples)
        state_manager.update(sample.first, sample.second);
    }
    void TearDown()
    {
      string rm_command = string("rm ") + kFilePath;
      system(rm_command.c_str());
    }
    static constexpr char kFilePath[] = ".temp_state";
    static constexpr size_t kFileSize = 4 * pow(2, 30);  // 1 GB

};

TEST_F(StateManagerStoringTest, retrieved_file_size_should_be_correct)
{
  static constexpr uint16_t kChunksMax = 5;
  const map<uint16_t, size_t> kSamples = {{0, 1234}};
  SetUp(kChunksMax, kSamples);
  StateManager state_manager(kFilePath);
  EXPECT_EQ(kFileSize, state_manager.get_file_size());
}

TEST_F(StateManagerStoringTest, retrieved_parts_should_be_correct)
{
  static constexpr uint16_t kChunksMax = 3;
  const map<uint16_t, size_t> kSamples = {
    {0, 1234},
    {1, 1345},
    {2, 1499}};
  SetUp(kChunksMax, kSamples);
  StateManager state_manager(kFilePath);
  vector<uint16_t> parts_list = state_manager.get_parts();
  EXPECT_EQ(parts_list.size(), kSamples.size());
  for (uint16_t i : parts_list)
    EXPECT_EQ(state_manager.get_current_pos(i),
              kSamples.at(i) + state_manager.get_start_pos(i));
}

TEST_F(StateManagerStoringTest, total_downloaded_bytes_should_be_correct)
{
  static constexpr uint16_t kChunksMax = 3;
  const map<uint16_t, size_t> kSamples = {
    {0, 1234},
    {1, 1345},
    {2, 1499}};
  SetUp(kChunksMax, kSamples);
  StateManager state_manager(kFilePath);
  vector<uint16_t> parts_list = state_manager.get_parts();
  EXPECT_EQ(parts_list.size(), kSamples.size());
  size_t total_bytes = 0;
  for (auto sample : kSamples)
    total_bytes += sample.second;
  EXPECT_EQ(total_bytes, state_manager.get_total_recvd_bytes());
}
