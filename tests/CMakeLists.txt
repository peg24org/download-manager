cmake_minimum_required (VERSION 2.6)
project (DownloadManagerTest)

find_library(GTEST gtest)
if (NOT GTEST)
       message(FATAL_ERROR "gtest library not found")
endif()

set (SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/url_info_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/url_info.cpp)

add_executable(url_info_test ${SOURCES})
target_link_libraries(url_info_test pthread gtest gtest_main)
