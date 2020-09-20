cmake_minimum_required (VERSION 2.6)
project (Downloader)

find_library(SSL ssl)
if (NOT SSL)
       message(FATAL_ERROR "ssl library not found")
endif()

set (Downloader_VERSION_MAJOR 1)
set (Downloader_VERSION_MINOR 0)
set (CMAKE_CXX_STANDARD 17)

set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/libs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/libs)

include_directories(${PROJECT_SOURCE_DIR} include)
include_directories(${PROJECT_SOURCE_DIR} thread/include)
add_subdirectory(src)
add_subdirectory(thread/src)

add_executable(main main.cpp )
target_link_libraries(main thread_static downloader_static pthread ssl crypto)

add_subdirectory(tests)
