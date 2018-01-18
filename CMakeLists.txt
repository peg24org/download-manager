cmake_minimum_required (VERSION 2.6)
project (Downloader)

set (Downloader_VERSION_MAJOR 1)
set (Downloader_VERSION_MINOR 0)
set (CMAKE_CXX_STANDARD 11)

SET(GCC_COVERAGE_COMPILE_FLAGS "-Wunused-variable ")
#SET(GCC_COVERAGE_LINK_FLAGS    "")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/libs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/libs)

include_directories(${PROJECT_SOURCE_DIR} include)
add_subdirectory(src)

add_executable(main main.cpp )
target_link_libraries(main downloader_static pthread)
