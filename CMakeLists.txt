cmake_minimum_required (VERSION 2.8)
project (Downloader)

find_library(SSL ssl)
if (NOT SSL)
       message(FATAL_ERROR "ssl library not found")
endif()

set (Downloader_VERSION_MAJOR 1)
set (Downloader_VERSION_MINOR 0)
set (CMAKE_CXX_STANDARD 17)

set(GXX_FLAGS "-Wall -g")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR} include)
include_directories(${PROJECT_SOURCE_DIR} thread/include)

add_subdirectory(src)
#add_subdirectory(test)
add_subdirectory(thread/src)

