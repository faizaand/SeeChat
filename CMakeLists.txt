cmake_minimum_required(VERSION 3.17)
project(SeeChat)

include_directories(/usr/local/include)
set(CMAKE_CXX_STANDARD 14)

# Server configuration
add_executable(SeeChat-Server server/server.cpp)

# Client configuration
set(FLTK_DIR /Users/faizaand/Developer/fltk/build)
find_package(FLTK REQUIRED NO_MODULE)
add_executable(SeeChat-Client WIN32 MACOSX_BUNDLE client/client.cpp)
if (APPLE)
    target_link_libraries(SeeChat-Client "-framework cocoa")
endif (APPLE)

target_include_directories(SeeChat-Client PUBLIC ${FLTK_INCLUDE_DIRS})
target_link_libraries(SeeChat-Client fltk)