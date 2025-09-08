# server/CMakeLists.txt

# --- 1. 查找服务器模块的依赖 ---
find_package(asio REQUIRED)
find_package(SOCI REQUIRED COMPONENTS mysql)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)


# --- 3. 定义服务器可执行文件 ---
#    使用 file(GLOB_RECURSE) 自动查找所有源文件
file(GLOB_RECURSE SERVER_SOURCES "src/*.cpp")
add_executable(server ${SERVER_SOURCES})




target_include_directories(server PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
# --- 5. 链接所有依赖 ---
target_link_libraries(server PRIVATE
    common
    asio::asio
    SOCI::soci_core
    SOCI::soci_mysql
    nlohmann_json::nlohmann_json
)
