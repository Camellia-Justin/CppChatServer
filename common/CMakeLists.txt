
# --- 1. 查找 common 模块自身的依赖 ---
find_package(Protobuf REQUIRED)

# --- 2. 处理 Protobuf 代码生成 ---
# 找到所有的 .proto 文件
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")

# 检查是否找到了 .proto 文件，这是一个好习惯
if(NOT PROTO_FILES)
    message(WARNING "No .proto files found in ${CMAKE_CURRENT_SOURCE_DIR}/proto/")
endif()

# 定义生成规则
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# --- 3. 收集手写的源文件 ---
# 即使现在没有 .cpp 文件，这个命令也会创建一个空的 COMMON_SOURCES 变量，
# 这在逻辑上是正确的。
file(GLOB_RECURSE COMMON_SOURCES "src/*.cpp")

# --- 4. 定义 common 库 ---
# 关键修改：将两个源文件列表都放进去
add_library(common STATIC 
    ${COMMON_SOURCES}  # 你手写的 .cpp 文件
    ${PROTO_SRCS}      # Protobuf 生成的 .pb.cc 文件
    ${PROTO_HDRS}      # 最好也把头文件加进去，方便 IDE 识别
)

# --- 5. 设置 common 库的“公开接口” ---
target_include_directories(common PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

# --- 6. 链接 common 库自身需要的私有依赖 ---
target_link_libraries(common PRIVATE ${Protobuf_LIBRARIES} )