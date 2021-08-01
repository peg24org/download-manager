file(GLOB SOURCES *.cpp)
add_library(downloader_static STATIC "${SOURCES}")
add_library(downloader_shared SHARED "${SOURCES}")

set_target_properties(downloader_static PROPERTIES OUTPUT_NAME downloader_static)
target_link_libraries(downloader_static ssl)
target_link_libraries(downloader_static crypto)

set_target_properties(downloader_shared PROPERTIES OUTPUT_NAME downlader_shared)
target_link_libraries(downloader_shared ssl)
target_link_libraries(downloader_shared crypto)

add_executable(demo main.cpp)

target_link_libraries(demo downloader_shared)
target_link_libraries(demo thread_static)
