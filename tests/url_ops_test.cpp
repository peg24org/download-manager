#include <tuple>
#include <string>

#include <gtest/gtest.h>

#include "url_ops.h"   // protocols

using namespace std;
using namespace testing;

constexpr char kInvalidUrl[] = "invalid url";
constexpr char kValidUrl[] = "ftp://example.com/dir/subdir/file.dat";

// <url> <file name> <port> <host name> <path> <protocol>
using TestPrams = tuple<string, string, uint16_t, string, string, Protocol>;

class UrlOpsTest : public TestWithParam<TestPrams>
{
};

INSTANTIATE_TEST_CASE_P(
    UrlOpsTests,
    UrlOpsTest, ::testing::Values(
      make_tuple("http://example.com/dir/subdir/file.dat", "file.dat",
                 80, "example.com", "/dir/subdir/", Protocol::HTTP),
      make_tuple("http://www.example.com/dir/subdir/file.dat", "file.dat",
                 80, "www.example.com", "/dir/subdir/", Protocol::HTTP),
      make_tuple("http://www.example.com/dir/file.dat", "file.dat",
                 80, "www.example.com", "/dir/", Protocol::HTTP),
      make_tuple("http://www.example.com/dir/subdir/file", "file",
                 80, "www.example.com", "/dir/subdir/", Protocol::HTTP),
      make_tuple("http://www.example.com:1234/dir/subdir/file", "file",
                 1234, "www.example.com", "/dir/subdir/", Protocol::HTTP),
      make_tuple("http://127.0.0.1/dir/subdir/file.dat", "file.dat",
                 80, "127.0.0.1", "/dir/subdir/", Protocol::HTTP),
      make_tuple("http://127.0.0.1:85/file.dat", "file.dat",
                 85, "127.0.0.1", "/", Protocol::HTTP),
      make_tuple("http://www.exam-ple.com:1234/dir/subdir/file", "file",
                 1234, "www.exam-ple.com", "/dir/subdir/", Protocol::HTTP),
      make_tuple("http://www.example.com:1234/di-r/sub-dir/fi-le", "fi-le",
                 1234, "www.example.com", "/di-r/sub-dir/", Protocol::HTTP),
      make_tuple("http://www.exam-p.le.com:1234/di-r/sub-dir/fi-le", "fi-le",
                 1234, "www.exam-p.le.com", "/di-r/sub-dir/", Protocol::HTTP),

      make_tuple("https://example.com/dir/subdir/file.dat", "file.dat",
                 443, "example.com", "/dir/subdir/", Protocol::HTTPS),
      make_tuple("https://www.example.com/dir/subdir/file.dat", "file.dat",
                 443, "www.example.com", "/dir/subdir/", Protocol::HTTPS),
      make_tuple("https://www.example.com/dir/file.dat", "file.dat",
                 443, "www.example.com", "/dir/", Protocol::HTTPS),
      make_tuple("https://www.example.com/dir/subdir/file", "file",
                 443, "www.example.com", "/dir/subdir/", Protocol::HTTPS),
      make_tuple("https://www.example.com:1234/dir/subdir/fi.le", "fi.le",
                 1234, "www.example.com", "/dir/subdir/", Protocol::HTTPS),
      make_tuple("https://127.0.0.1:1234/dir/subdir/fi.le", "fi.le",
                 1234, "127.0.0.1", "/dir/subdir/", Protocol::HTTPS),
      make_tuple("https://127.0.0.1:85/file.dat", "file.dat",
                 85, "127.0.0.1", "/", Protocol::HTTPS),
      make_tuple("https://www.exam-ple.com:1234/dir/subdir/file", "file",
                 1234, "www.exam-ple.com", "/dir/subdir/", Protocol::HTTPS),
      make_tuple("https://www.example.com:1234/di-r/sub-dir/fi-le", "fi-le",
                 1234, "www.example.com", "/di-r/sub-dir/", Protocol::HTTPS),
      make_tuple("https://www.exam-p.le.com:1234/di-r/sub-dir/fi-le", "fi-le",
                 1234, "www.exam-p.le.com", "/di-r/sub-dir/", Protocol::HTTPS),

      make_tuple("ftp://example.com/dir/subdir/file.dat", "file.dat",
                 21, "example.com", "/dir/subdir/", Protocol::FTP),
      make_tuple("ftp://www.example.com/dir/subdir/file.dat", "file.dat",
                 21, "www.example.com", "/dir/subdir/", Protocol::FTP),
      make_tuple("ftp://www.example.com/dir/file.dat", "file.dat",
                 21, "www.example.com", "/dir/", Protocol::FTP),
      make_tuple("ftp://www.example.com/dir/subdir/file", "file",
                 21, "www.example.com", "/dir/subdir/", Protocol::FTP),
      make_tuple("ftp://www.example.com:1234/dir/subdir/fi.le", "fi.le",
                 1234, "www.example.com", "/dir/subdir/", Protocol::FTP),
      make_tuple("ftp://127.0.0.1:1234/dir/subdir/fi.le", "fi.le",
                 1234, "127.0.0.1", "/dir/subdir/", Protocol::FTP),
      make_tuple("ftp://127.0.0.1:85/file.dat", "file.dat",
                 85, "127.0.0.1", "/", Protocol::FTP),
      make_tuple("ftp://www.exam-ple.com:1234/dir/subdir/file", "file",
                 1234, "www.exam-ple.com", "/dir/subdir/", Protocol::FTP),
      make_tuple("ftp://www.example.com:1234/di-r/sub-dir/fi-le", "fi-le",
                 1234, "www.example.com", "/di-r/sub-dir/", Protocol::FTP),
      make_tuple("ftp://www.exam-p.le.com:1234/di-r/sub-dir/fi-le", "fi-le",
                 1234, "www.exam-p.le.com", "/di-r/sub-dir/", Protocol::FTP)
      ));

TEST_P(UrlOpsTest, url_ops_should_return_right_values)
{
    string url = get<0>(GetParam());
    string expected_file_name = get<1>(GetParam());
    uint16_t expected_port = get<2>(GetParam());
    string expected_hostname = get<3>(GetParam());
    string expected_filepath = get<4>(GetParam());
    Protocol expected_protocol = get<5>(GetParam());

    UrlOps url_ops(url);

    ASSERT_EQ(expected_hostname, url_ops.get_hostname());
    ASSERT_EQ(expected_filepath, url_ops.get_path());
    ASSERT_EQ(expected_protocol, url_ops.get_protocol());
    ASSERT_EQ(expected_port, url_ops.get_port());
    ASSERT_EQ(expected_file_name, url_ops.get_file_name());
}

TEST(UrlOpsExceptionTest, url_ops_should_throw_exception_with_invalid_url)
{
  EXPECT_THROW(UrlOps invalid_url_ops(kInvalidUrl), invalid_argument);
}

TEST(UrlOpsProxyTest, after_set_proxy_url_ops_should_return_its_url_and_port)
{
  constexpr char kProxyUrl[] = "http://localhost:8080";
  constexpr char kExpectedIp[] = "127.0.0.1";
  constexpr uint16_t kExpectedPort = 8080;

  UrlOps url_ops(kValidUrl);
  url_ops.set_proxy(kProxyUrl);

  EXPECT_EQ(url_ops.get_proxy_port(), kExpectedPort);
  EXPECT_EQ(url_ops.get_proxy_ip(), kExpectedIp);
}

TEST(UrlOpsProxyTest, url_ops_should_throw_exception_if_proxy_is_invalid)
{
  UrlOps url_ops(kValidUrl);
  EXPECT_THROW(url_ops.set_proxy(kInvalidUrl), invalid_argument);
}
