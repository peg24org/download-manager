#include <gtest/gtest.h>

#include "transceiver.h"

using namespace std;

class TransceiverTest : public ::testing::Test
{
  void SetUp()
  {
  }
  protected:
};

TEST_F(TransceiverTest, buffer_should_able_treat_like_raw_buffer)
{
  Transceiver transv;
}
