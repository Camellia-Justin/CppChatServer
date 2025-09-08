# 顶层 CMakeLists.txt
# vcpkg Integration
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
    message(STATUS "Using vcpkg toolchain from ENV: ${CMAKE_TOOLCHAIN_FILE}")
endif()

cmake_minimum_required(VERSION 3.15)
project(ChatServer)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 假设你已经通过 -DCMAKE_TOOLCHAIN_FILE=... 传入了 vcpkg 的路径

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)