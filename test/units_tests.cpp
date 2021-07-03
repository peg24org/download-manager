#include <gtest/gtest.h>

#include "units.h"

TEST(UnitsOperatorTest, _KB_operator_should_return_input_x_2_pow_10)
{
  EXPECT_EQ(0, 0_KB);
  EXPECT_EQ(pow(2, 10), 1_KB);
  EXPECT_EQ(2 * pow(2, 10), 2_KB);
  EXPECT_EQ(3 * pow(2, 10), 3_KB);
  EXPECT_EQ(1000 * pow(2, 10), 1000_KB);
}

TEST(UnitsOperatorTest, _MB_operator_should_return_input_x_2_pow_20)
{
  EXPECT_EQ(0, 0_MB);
  EXPECT_EQ(pow(2, 20), 1_MB);
  EXPECT_EQ(2 * pow(2, 20), 2_MB);
  EXPECT_EQ(3 * pow(2, 20), 3_MB);
  EXPECT_EQ(1000 * pow(2, 20), 1000_MB);
}

TEST(UnitsOperatorTest, _GB_operator_should_return_input_x_2_pow_30)
{
  EXPECT_EQ(0, 0_GB);
  EXPECT_EQ(pow(2, 30), 1_GB);
  EXPECT_EQ(2 * pow(2, 30), 2_GB);
  EXPECT_EQ(3 * pow(2, 30), 3_GB);
  EXPECT_EQ(1000 * pow(2, 30), 1000_GB);
}

