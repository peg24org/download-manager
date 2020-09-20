file(GLOB SOURCES *.cpp)
add_library(downloader_static STATIC "${SOURCES}")
set_target_properties(downloader_static PROPERTIES OUTPUT_NAME downloader_static)
#set_target_properties(downloader_shared PROPERTIES OUTPUT_NAME downlader_shared)
