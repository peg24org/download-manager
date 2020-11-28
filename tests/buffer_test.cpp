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
    buffer.set_length(buffer.kDefaultCapacity);
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
  buffer.set_length(kLen);

  EXPECT_EQ(0, strncmp(data, buffer, kLen));
}

TEST_F(BufferTest, capacity_setter_should_set_values_correctly)
{
  constexpr static size_t kTestBufferSize = 2000;
  EXPECT_EQ(buffer.kDefaultCapacity, buffer.capacity());
  buffer.set_capacity(kTestBufferSize);

  EXPECT_EQ(kTestBufferSize, buffer.capacity());
}

TEST_F(BufferTest, buffer_after_resizing_should_have_correct_amount_of_storage)
{
  constexpr static size_t kTestBufferSize = buffer.kDefaultCapacity * 3;
  buffer.set_capacity(kTestBufferSize);

  unique_ptr<char[]> test_data = get_random_buffer(kTestBufferSize);
  memcpy(buffer, test_data.get(), kTestBufferSize);

  EXPECT_EQ(0, strncmp(test_data.get(), buffer, kTestBufferSize));
}

TEST_F(BufferTest, contents_of_two_buffer_should_be_same_after_copy)
{
  unique_ptr<char[]> test_data = get_random_buffer(buffer.kDefaultCapacity);
  memcpy(buffer, test_data.get(), buffer.kDefaultCapacity);
  buffer.set_length(buffer.kDefaultCapacity);

  Buffer buffer_1;
  buffer_1 = buffer;

  EXPECT_EQ(0, strncmp(buffer, buffer_1, buffer.kDefaultCapacity));
}

TEST_F(BufferTest, using_copy_assignment_should_copy_data_and_params)
{
  constexpr static size_t kLowCapacitySize = 1024;
  unique_ptr<char[]> test_data = get_random_buffer(buffer.kDefaultCapacity);
  memcpy(buffer, test_data.get(), buffer.kDefaultCapacity);
  buffer.set_length(buffer.kDefaultCapacity);

  Buffer buffer_1(kLowCapacitySize);
  buffer_1 = buffer;

  EXPECT_EQ(buffer_1.capacity(), buffer.capacity());
  EXPECT_EQ(buffer_1.length(), buffer.length());
  EXPECT_EQ(0, strncmp(buffer, buffer_1, buffer.kDefaultCapacity));
}

TEST_F(BufferTest, copy_constructor_should_copy_data_and_params)
{
  unique_ptr<char[]> test_data = get_random_buffer(buffer.kDefaultCapacity);
  memcpy(buffer, test_data.get(), buffer.kDefaultCapacity);
  buffer.set_length(buffer.kDefaultCapacity);

  Buffer buffer_1 = buffer;

  EXPECT_EQ(buffer_1.length(), buffer.length());
  EXPECT_EQ(buffer_1.capacity(), buffer.capacity());
  EXPECT_EQ(0, strncmp(buffer, buffer_1, buffer.kDefaultCapacity));
  EXPECT_STREQ(buffer, buffer_1);
}

TEST_F(BufferTest, move_ctor_should_move_and_deallocate_orig_data)
{
  unique_ptr<char[]> orig_data = make_unique<char[]>(buffer.length());
  memcpy(orig_data.get(), buffer, buffer.length());

  Buffer buffer_1 = move(buffer);
  const char* kOldData = buffer;

  EXPECT_EQ(nullptr, kOldData);
  EXPECT_EQ(buffer.length(), buffer_1.length());
  EXPECT_EQ(buffer.capacity(), buffer_1.capacity());
  EXPECT_STREQ(orig_data.get(), buffer_1);
}

TEST_F(BufferTest, move_assignment_should_move_and_deallocate_orig_data)
{
  unique_ptr<char[]> orig_data = make_unique<char[]>(buffer.length());
  memcpy(orig_data.get(), buffer, buffer.length());

  Buffer buffer_1;
  buffer_1 = move(buffer);
  const char* kOldData = buffer;

  EXPECT_EQ(nullptr, kOldData);
  EXPECT_EQ(buffer.length(), buffer_1.length());
  EXPECT_EQ(buffer.capacity(), buffer_1.capacity());
  EXPECT_STREQ(orig_data.get(), buffer_1);
}

TEST_F(BufferTest, clear_should_set_length_to_zero_and_not_change_the_contents)
{
  unique_ptr<char[]> test_data = make_unique<char[]>(buffer.length());
  memcpy(test_data.get(), buffer, buffer.length());

  EXPECT_NE(0, buffer.length());
  
  buffer.clear();

  EXPECT_STREQ(test_data.get(), buffer);
  EXPECT_EQ(0, buffer.length());
}


TEST_F(BufferTest, deep_clear_should_set_length_to_zero_and_contents_to_null)
{
  unique_ptr<char[]> test_data = make_unique<char[]>(buffer.length());
  memset(test_data.get(), '\0', buffer.length());

  EXPECT_NE(0, buffer.length());
  
  buffer.deep_clear();

  EXPECT_STREQ(test_data.get(), buffer);
  EXPECT_EQ(0, buffer.length());
}

TEST_F(BufferTest, extending_the_buffer_should_not_change_the_contents_and_len)
{
  unique_ptr<char[]> old_contents = make_unique<char[]>(buffer.length());
  size_t old_len = buffer.length();
  memcpy(old_contents.get(), buffer, buffer.length());

  buffer.extend(old_len * 3);

  size_t new_len = buffer.length();

  EXPECT_STREQ(old_contents.get(), buffer);
  EXPECT_EQ(old_len, new_len);
}

TEST_F(BufferTest, extending_the_buffer_should_increase_capasity)
{
  size_t old_len = buffer.length();
  size_t old_cap = buffer.capacity();
  size_t new_cap = old_cap * 4;

  buffer.extend(new_cap);

  size_t cap = buffer.capacity();
  size_t len = buffer.length();

  EXPECT_EQ(cap, new_cap);
  EXPECT_NE(cap, old_cap);
  EXPECT_EQ(len, old_len);
}

TEST_F(BufferTest, c_str_insertion_operator_should_insert_data_and_len)
{
  Buffer test_buffer;
  unique_ptr<char[]> test_data;
  test_data = get_random_buffer(70000, 0x21, 0x7e); // Printable characters
  size_t test_data_len = strlen(test_data.get());

  EXPECT_STRNE(test_data.get(), test_buffer);
  EXPECT_EQ(0, test_buffer.length());

  test_buffer << test_data.get();

  size_t len = test_buffer.length();

  EXPECT_EQ(len, test_data_len);
  EXPECT_STREQ(test_data.get(), test_buffer);
}

TEST_F(BufferTest, c_str_insertion_operator_should_append_the_end_of_contents)
{
  unique_ptr<char[]> random_str;
  size_t kRandLen = 50000;
  random_str = get_random_buffer(kRandLen, 0x20, 0x7e);
  size_t expected_len = strlen(random_str.get()) + buffer.length();

  unique_ptr<char[]> expected_contents = make_unique<char[]>(expected_len);
  memcpy(expected_contents.get(), buffer, buffer.length());
  memcpy(expected_contents.get()+buffer.length(), random_str.get(), kRandLen);

  EXPECT_NE(0, buffer.length());

  buffer << random_str.get();

  EXPECT_EQ(expected_len, buffer.length());
  EXPECT_STREQ(expected_contents.get(), buffer);
}

TEST_F(BufferTest, insertion_operator_should_insert_data_and_len_correctly)
{
  Buffer test_buffer;
  unique_ptr<char[]> test_data;
  test_data = get_random_buffer(70000, 0x20, 0x7e);
  string test_str(test_data.get(), strlen(test_data.get()));

  EXPECT_STRNE(test_str.c_str(), test_buffer);
  EXPECT_EQ(0, test_buffer.length());

  test_buffer << test_str;

  EXPECT_EQ(test_buffer.length(), test_str.length());
  EXPECT_STREQ(test_str.c_str(), test_buffer);
}

TEST_F(BufferTest, insertion_operator_should_insert_data_to_the_end_of_contents)
{
  unique_ptr<char[]> random_str;
  size_t kRandLen = 50000;
  random_str = get_random_buffer(kRandLen, 0x20, 0x7e);
  string test_str(random_str.get(), strlen(random_str.get()));
  size_t expected_len = test_str.length() + buffer.length();

  unique_ptr<char[]> expected_contents = make_unique<char[]>(expected_len);
  memcpy(expected_contents.get(), buffer, buffer.length());
  memcpy(expected_contents.get()+buffer.length(), test_str.c_str(), kRandLen);

  EXPECT_NE(0, buffer.length());

  buffer << test_str;

  EXPECT_EQ(expected_len, buffer.length());
  EXPECT_STREQ(expected_contents.get(), buffer);
}

TEST_F(BufferTest, buffer_should_preserve_inserted_str_literals_order)
{
  Buffer test_buffer;

  test_buffer << "one" << " " << "two";

  EXPECT_STREQ("one two", test_buffer);
}

TEST_F(BufferTest, buffer_should_preserve_inserted_cstrs_order)
{
  Buffer test_buffer;

  char one[] = "one";
  char space[] = " ";
  char two[] = "two";
  string expected_string = string(one) + string(space) + string(two);

  test_buffer << one << space << two;

  EXPECT_STREQ(expected_string.c_str(), test_buffer);
}

TEST_F(BufferTest, buffer_should_preserve_inserted_strings_order)
{
  Buffer test_buffer;

  string one("one");
  string space(" ");
  string two("two");
  string expected_string = one + space + two;

  test_buffer << one << space << two;

  EXPECT_STREQ(expected_string.c_str(), test_buffer);
}

TEST_F(BufferTest, buffer_should_preserve_inserted_chars_order)
{
  Buffer test_buffer;
  constexpr char kTestData[] = "test";

  test_buffer << kTestData[0];
  test_buffer << kTestData[1] << kTestData[2] << kTestData[3];

  EXPECT_STREQ(kTestData, test_buffer);
}

TEST_F(BufferTest, inserting_char_should_append_char_to_the_end_of_data)
{
  constexpr char kTestChar = 'X';
  const string expected_data = string(buffer, buffer.length()) + kTestChar;

  buffer << kTestChar;

  EXPECT_STREQ(expected_data.c_str(), buffer);
}
