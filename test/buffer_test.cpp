#include <gtest/gtest.h>

#include <cstring>

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
  Buffer cap_test_buffer;
  constexpr static size_t kTestBufferSize = 2000;
  EXPECT_EQ(Buffer::kDefaultCapacity, cap_test_buffer.capacity());
  cap_test_buffer.set_capacity(kTestBufferSize);

  EXPECT_EQ(kTestBufferSize, cap_test_buffer.capacity());
  EXPECT_EQ(0, cap_test_buffer.length());
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

TEST_F(BufferTest, copy_assignment_should_copy_data_and_params)
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
  size_t old_cap = buffer.capacity();
  memcpy(old_contents.get(), buffer, buffer.length());

  buffer.extend((old_len + old_cap) * 3);

  size_t new_len = buffer.length();

  EXPECT_STREQ(old_contents.get(), buffer);
  EXPECT_EQ(old_len, new_len);
}

TEST_F(BufferTest, extending_the_buffer_should_increase_capacity)
{
  size_t old_len = buffer.length();
  size_t old_cap = buffer.capacity();
  size_t new_cap = (old_cap + old_len) * 4;

  buffer.extend(new_cap);

  size_t cap = buffer.capacity();
  size_t len = buffer.length();

  EXPECT_EQ(cap, new_cap - len);
  EXPECT_NE(cap, old_cap);
  EXPECT_EQ(len, old_len);
}

TEST_F(BufferTest, buffer_should_preserve_inserted_str_literals_order)
{
  Buffer test_buffer;

  test_buffer << "one" << " " << "two";

  EXPECT_STREQ("one two", test_buffer);
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

template<typename T>
struct BufferInsertionOperatorTest : public testing::Test
{
  using ParamType = T;
  void SetUp()
  {
    unique_ptr<char[]> test_data = get_random_buffer(buffer.kDefaultCapacity);
    memcpy(buffer, test_data.get(), buffer.kDefaultCapacity);
    buffer.set_length(buffer.kDefaultCapacity);
  }
  Buffer buffer;
  size_t rand_len = 70000;
};

using MyTypes = testing::Types<string, char*, char, int>;
TYPED_TEST_CASE(BufferInsertionOperatorTest, MyTypes);

template<typename T>
size_t insert_data(Buffer& buffer, char* random_data)
{
  size_t insertion_len = 0;
  if (typeid(T) == typeid(string)) {
    string data(random_data, strlen(random_data));
    buffer << data;
    insertion_len = data.length();
  }
  else if(typeid(T) == typeid(char*)) {
    buffer << random_data;
    insertion_len = strlen(random_data);
  }
  else if(typeid(T) == typeid(char) || typeid(T) == typeid(int)) {
    buffer << random_data;
    insertion_len = strlen(random_data);
  }

  return insertion_len;
}

TYPED_TEST(BufferInsertionOperatorTest, should_insert_data_and_update_length)
{
  using ParamType  = typename TestFixture::ParamType;

  if (typeid(ParamType) == typeid(int) || typeid(ParamType) == typeid(char))
    this->rand_len = 1;

  Buffer test_buffer;
  unique_ptr<char[]> random_data;
  random_data = get_random_buffer(this->rand_len, 0x21, 0x7e); // Printable characters

  EXPECT_EQ(0, test_buffer.length());
  EXPECT_STRNE(random_data.get(), test_buffer);

  size_t test_data_len = strlen(random_data.get());
  insert_data<ParamType>(test_buffer, random_data.get());
  size_t len = test_buffer.length();

  EXPECT_EQ(len, test_data_len);
  EXPECT_STREQ(random_data.get(), test_buffer);
}

TYPED_TEST(BufferInsertionOperatorTest, should_append_the_end_of_contents)
{
  using ParamType  = typename TestFixture::ParamType;

  if (typeid(ParamType) == typeid(int) || typeid(ParamType) == typeid(char))
    this->rand_len = 1;

  // Create random data
  unique_ptr<char[]> random_data;
  random_data = get_random_buffer(this->rand_len, 0x20, 0x7e); // Printable chars
  size_t expected_contents_len = strlen(random_data.get()) + this->buffer.length();
  const size_t initial_buffer_len = this->buffer.length();

  // Create expected data
  unique_ptr<char[]> expected_contents = make_unique<char[]>(expected_contents_len);
  memcpy(expected_contents.get(), this->buffer, this->buffer.length());
  memcpy(expected_contents.get()+this->buffer.length(), random_data.get(), this->rand_len);

  EXPECT_NE(0, this->buffer.length());

  const size_t insertion_len = insert_data<ParamType>(this->buffer,
                                                      random_data.get());

  EXPECT_EQ(insertion_len+initial_buffer_len, this->buffer.length());
  EXPECT_STREQ(expected_contents.get(), this->buffer);
}

