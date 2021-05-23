cmake_minimum_required (VERSION 2.6)
project (DownloadManagerTest)

find_library(GTEST gtest)
if (NOT GTEST)
       message(FATAL_ERROR "gtest library not found")
endif()

# Add sources
set (SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/buffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/url_parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/socket_ops.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/https_socket_ops.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/connection_manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/state_manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/pattern_finder.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/transceiver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/file_io.cpp)

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
target_link_libraries(unit_tests gtest gtest_main pthread ssl crypto)

