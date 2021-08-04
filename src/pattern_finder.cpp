#include "pattern_finder.h"

#include <regex>
#include <cstring>
#include <sstream>

using namespace std;

bool PatternFinder::find_http_header(const string& buffer)
{
  return true;
//  static constexpr char kHttpHeader[] =
//    "(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)";
//  string output;
//
//  return regex_search_string(buffer, kHttpHeader, output, 2);
}

size_t PatternFinder::find_http_header_delimiter(const Buffer& buffer)
{
  size_t result = 0;
  static constexpr char kHeaderDelimiter[] = "\r\n\r\n";
  char* buffer_str = const_cast<Buffer&>(buffer);
  const char* pos = strstr(buffer_str, kHeaderDelimiter);
  if (pos != nullptr)
    result = pos - buffer_str;

  return result;
}

size_t PatternFinder::find_file_length(const Buffer& buffer)
{
  size_t file_length = 0;
  stringstream header;
  header << static_cast<char*>(const_cast<Buffer&>(buffer));
  string content;
  while (header >> content)
    if (content == "Content-Length:") {
      header >> file_length;
      break;
    }

  return file_length;
}

pair<bool, string> PatternFinder::find_redirection(const Buffer& buffer)
{
  string buffer_str = string(const_cast<Buffer&>(buffer), buffer.length());
  return find_redirection(buffer_str);
}

pair<bool, string> PatternFinder::find_redirection(const string& buffer)
{
  pair<bool, string> result;
  result.first = false;
  string location;
  const string kLocationStr = "Location:";
  const size_t kLocStrLen = kLocationStr.length();
  const size_t kStrPos = buffer.find(kLocationStr);

  if (kStrPos != string::npos) {
    result.first = true;
    const string kDelimiter =   "\r\n";
    const size_t kStrEnd = buffer.find(kDelimiter, kStrPos);
    const size_t kStrLength = kStrEnd - (kStrPos + kLocStrLen - 1);
    result.second = buffer.substr(kStrPos + kLocStrLen + 1, kStrLength - 2);
  }

  return result;
}

bool PatternFinder::regex_search_string(const string& input,
                                     const string& pattern,
                                     string& output, int pos_of_pattern)
{
  smatch match;
  regex e(pattern);
  bool retval = regex_search(input, match, e);
  output = match[pos_of_pattern];

  return retval;
}

