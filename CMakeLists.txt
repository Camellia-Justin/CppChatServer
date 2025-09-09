# 顶层 CMakeLists.txt (最终修正版)
cmake_minimum_required(VERSION 3.20)

# --- 关键：在 project() 之前设置工具链 ---
# vcpkg 会自动处理所有平台差异
set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")


project(ChatServer LANGUAGES CXX)

if(WIN32)
    # 0x0A00 对应 Windows 10
    # 这是一个现代、安全的选择
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()
set(CMAKE_CXX_STANDARD 17) # 建议先使用 C++17，更稳定
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置统一的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# 包含子项目
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(tester)