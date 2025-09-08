# client/CMakeLists.txt (最终修正版)

# 只查找 client 自身直接需要的依赖
find_package(asio CONFIG REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE CLIENT_SOURCES "src/*.cpp")
add_executable(client ${CLIENT_SOURCES})

target_include_directories(client PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(client PRIVATE
    common      # common 会自动带来 protobuf 的链接
    asio::asio
    Threads::Threads
)

if(WIN32)
    target_link_libraries(client PRIVATE ws2_32)
endif()