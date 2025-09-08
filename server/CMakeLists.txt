# server/CMakeLists.txt (最终修正版)

# 只查找 server 自身直接需要的依赖
find_package(asio CONFIG REQUIRED)
find_package(SOCI CONFIG REQUIRED COMPONENTS mysql)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SERVER_SOURCES "src/*.cpp")
add_executable(server ${SERVER_SOURCES})

# 只需要包含自己的 src 目录
target_include_directories(server PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
     ${CMAKE_BINARY_DIR}
     "${CMAKE_BINARY_DIR}/common"
)

# 链接依赖，使用 vcpkg 提供的导入目标
target_link_libraries(server PRIVATE
    common      # common 会自动带来 protobuf 的链接
    asio::asio
    SOCI::soci_core
    SOCI::soci_mysql
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

# Windows 平台特定链接
if(WIN32)
    target_link_libraries(server PRIVATE ws2_32)
endif()