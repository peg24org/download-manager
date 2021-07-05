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

target_link_libraries(unit_tests ssl)
target_link_libraries(unit_tests crypto)
target_link_libraries(unit_tests gtest)
target_link_libraries(unit_tests gtest_main)
target_link_libraries(unit_tests downloader_shared)
target_link_libraries(unit_tests thread_static)


