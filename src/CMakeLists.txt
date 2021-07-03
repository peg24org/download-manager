file(GLOB SOURCES *.cpp)
add_library(downloader_static STATIC "${SOURCES}")
set_target_properties(downloader_static PROPERTIES OUTPUT_NAME downloader_static)
#set_target_properties(downloader_shared PROPERTIES OUTPUT_NAME downlader_shared)

add_executable(demo main.cpp)
target_link_libraries(demo thread_static pthread ssl crypto downloader_static)

