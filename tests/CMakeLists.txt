cmake_minimum_required (VERSION 2.6)
project (DownloadManagerTest)

find_library(GTEST gtest)
if (NOT GTEST)
       message(FATAL_ERROR "gtest library not found")
endif()

# Add sources
set (SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/writer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/download_state_manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/file_io.cpp)

# Add unit tests
set (SOURCES ${SOURCES}
  writer_test.cpp
  file_io_test.cpp
  download_state_manager_test.cpp)

add_executable(unit_tests test_utils.h ${SOURCES})
target_link_libraries(unit_tests gtest gtest_main pthread)
