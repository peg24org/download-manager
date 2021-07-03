cmake_minimum_required (VERSION 2.8)
project (TEST)

find_library(GTEST gtest)
if (NOT GTEST)
       message(FATAL_ERROR "gtest library not found")
endif()

# Add unit tests
set (SOURCES ${SOURCES}
  test_utils.cpp
  units_tests.cpp
  buffer_test.cpp
  url_parser_test.cpp
  connection_manager_test.cpp
  file_io_test.cpp
  transceiver_test.cpp
  state_manager_test.cpp)

add_executable(unit_tests ${SOURCES})
target_link_libraries(unit_tests gtest gtest_main pthread ssl crypto downloader_static)

