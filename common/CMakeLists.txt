# common/CMakeLists.txt

# --- 关键修改：我们使用 find_package 的两种模式 ---

# 1. 首先，使用现代 CONFIG 模式找到 vcpkg 提供的库。
#    这会创建 protobuf::libprotobuf 等目标。
find_package(protobuf CONFIG REQUIRED)

# 2. 然后，为了获取 protobuf_generate_cpp 函数，我们再次调用 find_package，
#    但这次使用传统的 Module 模式。
#    因为第一次调用已经找到了库，这次调用主要是为了加载 FindProtobuf.cmake 模块
#    并定义相关的函数。
find_package(Protobuf REQUIRED)

# --- 现在，protobuf_generate_cpp 函数和 protobuf::* 目标应该都可用了 ---

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "proto/chat.proto")
file(GLOB_RECURSE COMMON_SOURCES "src/*.cpp")

add_library(common STATIC 
    ${COMMON_SOURCES}
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_include_directories(common PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${Protobuf_INCLUDE_DIRS}
)

# 链接时，我们优先使用现代 CONFIG 模式找到的导入目标
target_link_libraries(common INTERFACE protobuf::libprotobuf)