#include <tuple>
#include <cmath>
#include <memory>
#include <exception>

#include <gtest/gtest.h>

#include "file_io.h"
#include "test_utils.h"
#include "state_manager.h"

using namespace std;

class StateManagerTestClass : public StateManager
{
  public:
  StateManagerTestClass()
    : StateManager("null path")
    {
      state_file = make_unique<FileIOMock>();
    }

  void set_raw_stat_data(string data)
  {
    state_file->create(data.length());
    state_file->write(data.c_str(), data.length(), 0);
  }

  FileIOMock* get_file_io() const
  {
    return dynamic_cast<FileIOMock*>(state_file.get());
  }
};

class StateManagerTest : public ::testing::Test
{
  void SetUp()
  {
    state_manager.create_new_state(kFileSize);
  }

  protected:
    static constexpr size_t kFileSize = pow(2, 30);  // 1 GB
    StateManagerTestClass state_manager;
};

TEST_F(StateManagerTest,
    retrieving_from_non_existing_file_should_throw_exception)
{
  state_manager.get_file_io()->set_existence(false);
  EXPECT_THROW(state_manager.retrieve(), runtime_error);
}

TEST_F(StateManagerTest, set_chunk_size_should_work_properly)
{
  constexpr size_t kNewChunkSize = pow(2, 20) + 1;
  constexpr size_t kSmallerThanMinimum = 100;

  state_manager.set_chunk_size(kSmallerThanMinimum);
  EXPECT_NE(kSmallerThanMinimum, state_manager.get_chunk_size());

  state_manager.set_chunk_size(kNewChunkSize);
  EXPECT_EQ(kNewChunkSize, state_manager.get_chunk_size());
}

TEST_F(StateManagerTest, size_of_part_should_be_equal_to_current_chunk_size)
{
  size_t chunk_size = state_manager.get_chunk_size();
  pair<size_t, Chunk> part = state_manager.get_part();
  size_t part_size = part.second.end - part.second.start;
  EXPECT_EQ(chunk_size, part_size);
}

TEST_F(StateManagerTest, new_chunk_should_start_from_last_chunk)
{
  // -2 for creating non-dividable chunk
  constexpr size_t kFileSize = pow(2, 30) - 2;

  state_manager.create_new_state(kFileSize);
  vector<pair<size_t, Chunk>> positions_vecror;

  while (state_manager.part_available())
    positions_vecror.push_back(state_manager.get_part());

  for (auto position = positions_vecror.begin(); position != positions_vecror.end(); ++position) {
    static bool first_position = true;
    if (!first_position) {
      auto previous_position = position - 1;
      size_t end_of_prev_pos = previous_position->second.end;
      EXPECT_EQ(end_of_prev_pos, position->second.current);
    }
    else
      first_position = false;
  }
  
  EXPECT_EQ(kFileSize, (positions_vecror.end()-1)->second.end);
}

TEST_F(StateManagerTest, state_file_available_should_return_correct_value)
{
  state_manager.get_file_io()->set_existence(true);
  EXPECT_TRUE(state_manager.state_file_available());

  state_manager.get_file_io()->set_existence(false);
  EXPECT_FALSE(state_manager.state_file_available());
}

void store_fake_data(vector<tuple<size_t, size_t, size_t, size_t>>& data,
                     FileIOMock* file_io, size_t kFileSize)
{
  string state_data;
  state_data = to_string(kFileSize) + "\n";
  for (auto datum  : data) {
    string index = to_string(get<0>(datum));
    string start = to_string(get<1>(datum));
    string current = to_string(get<2>(datum));
    string end = to_string(get<3>(datum));

    state_data += index + " " + start + " " + current + " " + end + "\n";
  }

  file_io->create(state_data.size());
  file_io->write(state_data.c_str(), state_data.size());
  file_io->set_existence(true);
}

TEST_F(StateManagerTest, retrived_state_file_contents_check_0)
{

  constexpr size_t kFileSize = 3592673;
  vector<tuple<size_t, size_t, size_t, size_t>> data = {
    {0 ,0 ,398088 ,1167557},
    {1 ,1167558 ,1687398 ,2335114},
    {2 ,2335115 ,2599139 ,3502673},
    {3 ,3502673 ,3502828 ,3552673}
  };

  FileIOMock* file_io = state_manager.get_file_io();
  store_fake_data(data, file_io, kFileSize);
  state_manager.retrieve();

  pair<size_t, Chunk> last_part;
  while(true) {
    try {
      last_part = state_manager.get_part();
    }
    catch (const runtime_error& e) {
      break;
    }
  }

  auto last_datum = data.end() - 1;
  EXPECT_EQ(last_part.second.current, get<3>(*last_datum));
}

TEST_F(StateManagerTest, retrived_state_file_contents_check_1)
{
  FileIOMock* file_io = state_manager.get_file_io();

  constexpr size_t kFileSize = 3592673;
  vector<tuple<size_t, size_t, size_t, size_t>> data = {
    {0 ,0 ,398088 ,1167557},
    {1 ,1167558 ,1687398 ,2335114},
    {2 ,2335115 ,2599139 ,3502673},
    {3 ,3502673 ,3502828 ,3592673}
  };

  store_fake_data(data, file_io, kFileSize);
  state_manager.retrieve();

  while(state_manager.part_available()) {
    pair<size_t, Chunk> part = state_manager.get_part();
    size_t index = part.first;
    Chunk& chunk = part.second;
    EXPECT_EQ(get<2>(data.at(index)), chunk.current);
  }
}

TEST(ChunkNotAvailableTest, chunk_should_be_available_by_creating_new_state)
{
  static constexpr size_t kFileSize = pow(2, 30);  // 1 GB
  StateManagerTestClass state_manager;
  EXPECT_FALSE(state_manager.part_available());

  state_manager.create_new_state(kFileSize);
  EXPECT_TRUE(state_manager.part_available());
}
