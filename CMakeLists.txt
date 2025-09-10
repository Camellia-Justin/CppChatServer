# 顶层 CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")


project(ChatServer LANGUAGES CXX)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(tester)
