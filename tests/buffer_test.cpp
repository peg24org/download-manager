#include <gtest/gtest.h>

#include <cstring>
#include <iostream>

#include "buffer.h"
#include "test_utils.h"

using namespace std;

class BufferTest : public ::testing::Test
{
  void SetUp()
  {
    unique_ptr<char[]> test_data = get_random_buffer(buffer.kDefaultCapacity);
    memcpy(buffer, test_data.get(), buffer.kDefaultCapacity);
    buffer.length = buffer.kDefaultCapacity;
  }

  protected:
    Buffer buffer;
};

TEST_F(BufferTest, buffer_should_able_treat_like_raw_buffer)
{
  constexpr static size_t kLen = 2000;
  unique_ptr<char[]> test_data = get_random_buffer(kLen);
  char* data = test_data.get();
  memcpy(buffer, data, kLen);
  buffer.length = kLen;

  EXPECT_EQ(0, strncmp(data, buffer, kLen));
}

TEST_F(BufferTest, capacity_setter_should_set_values_correctly)
{
  constexpr static size_t kTestBufferSize = 2000;
  EXPECT_EQ(buffer.kDefaultCapacity, buffer.capacity());
  buffer.resize(kTestBufferSize);

  EXPECT_EQ(kTestBufferSize, buffer.capacity());
}

TEST_F(BufferTest, buffer_after_resizing_should_have_correct_amount_of_storage)
{
  constexpr static size_t kTestBufferSize = buffer.kDefaultCapacity * 3;
  buffer.resize(kTestBufferSize);

  unique_ptr<char[]> test_data = get_random_buffer(kTestBufferSize);
  memcpy(buffer, test_data.get(), kTestBufferSize);

  EXPECT_EQ(0, strncmp(test_data.get(), buffer, kTestBufferSize));
}

TEST_F(BufferTest, contents_of_two_buffer_should_be_same_after_copy)
{
  unique_ptr<char[]> test_data = get_random_buffer(buffer.kDefaultCapacity);
  memcpy(buffer, test_data.get(), buffer.kDefaultCapacity);
  buffer.length = buffer.kDefaultCapacity;

  Buffer buffer_1;
  buffer_1 = buffer;

  EXPECT_EQ(0, strncmp(buffer, buffer_1, buffer.kDefaultCapacity));
}

TEST_F(BufferTest, using_copy_assignment_should_copy_data_and_params)
{
  constexpr static size_t kLowCapacitySize = 1024;
  unique_ptr<char[]> test_data = get_random_buffer(buffer.kDefaultCapacity);
  memcpy(buffer, test_data.get(), buffer.kDefaultCapacity);
  buffer.length = buffer.kDefaultCapacity;

  Buffer buffer_1(kLowCapacitySize);
  buffer_1 = buffer;

  EXPECT_EQ(buffer_1.capacity(), buffer.capacity());
  EXPECT_EQ(buffer_1.length, buffer.length);
  EXPECT_EQ(0, strncmp(buffer, buffer_1, buffer.kDefaultCapacity));
}

TEST_F(BufferTest, copy_constructor_should_copy_data_and_params)
{
  unique_ptr<char[]> test_data = get_random_buffer(buffer.kDefaultCapacity);
  memcpy(buffer, test_data.get(), buffer.kDefaultCapacity);
  buffer.length = buffer.kDefaultCapacity;

  Buffer buffer_1 = buffer;

  EXPECT_EQ(buffer_1.length, buffer.length);
  EXPECT_EQ(buffer_1.capacity(), buffer.capacity());
  EXPECT_EQ(0, strncmp(buffer, buffer_1, buffer.kDefaultCapacity));
  EXPECT_STREQ(buffer, buffer_1);
}

TEST_F(BufferTest, move_ctor_should_move_and_deallocate_orig_data)
{
  unique_ptr<char[]> orig_data = make_unique<char[]>(buffer.length);
  memcpy(orig_data.get(), buffer, buffer.length);

  Buffer buffer_1 = move(buffer);
  const char* kOldData = buffer;

  EXPECT_EQ(nullptr, kOldData);
  EXPECT_EQ(buffer.length, buffer_1.length);
  EXPECT_EQ(buffer.capacity(), buffer_1.capacity());
  EXPECT_STREQ(orig_data.get(), buffer_1);
}

TEST_F(BufferTest, move_assignment_should_move_and_deallocate_orig_data)
{
  unique_ptr<char[]> orig_data = make_unique<char[]>(buffer.length);
  memcpy(orig_data.get(), buffer, buffer.length);

  Buffer buffer_1;
  buffer_1 = move(buffer);
  const char* kOldData = buffer;

  EXPECT_EQ(nullptr, kOldData);
  EXPECT_EQ(buffer.length, buffer_1.length);
  EXPECT_EQ(buffer.capacity(), buffer_1.capacity());
  EXPECT_STREQ(orig_data.get(), buffer_1);
}
