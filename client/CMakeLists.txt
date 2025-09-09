# client/CMakeLists.txt (最终修正版)

# 只查找 client 自身直接需要的依赖
find_package(asio CONFIG REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE CLIENT_LIB_SOURCES 
    "src/*.cpp"
    "src/*.h" 
)
list(FILTER CLIENT_LIB_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")
add_library(client_lib STATIC ${CLIENT_LIB_SOURCES})

target_include_directories(client_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(client_lib PUBLIC
    common      # common 会自动带来 protobuf 的链接
    asio::asio
    Threads::Threads
)

if(WIN32)
    target_link_libraries(client_lib PUBLIC ws2_32)
endif()

add_executable(client "src/main.cpp")
target_link_libraries(client PRIVATE client_lib)
