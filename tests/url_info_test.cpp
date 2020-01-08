#include <memory>
#include <string>
#include <gtest/gtest.h>

#include "url_info.h"

using namespace std;

class URLInfoTest : public ::testing::Test
{
  protected:
    struct addr_struct get_info(const string url)
    {
      URLInfo url_info(url);
      return url_info.get_download_info();
    }
};

TEST_F(URLInfoTest, should_get_default_http_port_number_from_url)
{
  constexpr static int DEFAULT_HTTP_PORT = 80;
  constexpr char URL_0[] = "http://www.example.com/file.dat";
  constexpr char URL_1[] = "http://example.com/file.dat";

  struct addr_struct info = get_info(URL_0);
  EXPECT_EQ(DEFAULT_HTTP_PORT, info.port);

  info = get_info(URL_1);
  EXPECT_EQ(DEFAULT_HTTP_PORT, info.port);
}

TEST_F(URLInfoTest, should_get_non_default_http_port_from_url)
{
  constexpr static int NON_DEFAULT_HTTP_PORT = 1234;
  constexpr char URL[] = "http://www.example.com:1234/file.dat";

  struct addr_struct info = get_info(URL);

  EXPECT_EQ(NON_DEFAULT_HTTP_PORT, info.port);
}

TEST_F(URLInfoTest, should_get_default_https_port_number_from_url)
{
  constexpr static int DEFAULT_HTTP_PORT = 443;
  constexpr char URL[] = "https://www.example.com/file.dat";

  struct addr_struct info = get_info(URL);

  EXPECT_EQ(DEFAULT_HTTP_PORT, info.port);
}

TEST_F(URLInfoTest, should_get_non_default_https_port_from_url)
{
  constexpr static int NON_DEFAULT_HTTP_PORT = 1234;
  constexpr char URL[] = "https://www.example.com:1234/file.dat";

  struct addr_struct info = get_info(URL);

  EXPECT_EQ(NON_DEFAULT_HTTP_PORT, info.port);
}
