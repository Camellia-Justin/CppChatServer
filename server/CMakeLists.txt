# server/CMakeLists.txt

# --- 1. 查找服务器模块的依赖 ---
find_package(asio REQUIRED)
find_package(SOCI REQUIRED COMPONENTS mysql)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Protobuf REQUIRED)


# --- 3. 定义服务器可执行文件 ---
#    使用 file(GLOB_RECURSE) 自动查找所有源文件
file(GLOB_RECURSE SERVER_SOURCES "src/*.cpp")
add_executable(server ${SERVER_SOURCES})




target_include_directories(server PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    ${asio_INCLUDE_DIRS}
    ${SOCI_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
)
# --- 5. 链接所有依赖 ---
target_link_libraries(server PUBLIC
    common  
    asio::asio
    ${SOCI_LIBRARIES}
    nlohmann_json::nlohmann_json
    Threads::Threads
    ${Protobuf_LIBRARIES}
)
if(WIN32)
    target_link_libraries(server PRIVATE ws2_32 
        wsock32)
endif()
