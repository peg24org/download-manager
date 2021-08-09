#ifndef _PATTERN_FINDER_H
#define _PATTERN_FINDER_H

#include "buffer.h"

class PatternFinder
{
  public:
    size_t find_http_header_delimiter(const Buffer& buffer);
    size_t find_file_length(const Buffer& buffer);
    std::pair<bool, std::string> find_redirection(const std::string& buffer);
    std::pair<bool, std::string> find_redirection(const Buffer& buffer);

  private:
    bool regex_search_string(const std::string& input,
                             const std::string& pattern,
                             std::string& output, int pos_of_pattern);
};

#endif

