# client/CMakeLists.txt

find_package(asio REQUIRED)

# --- 2. 自动查找所有源文件 ---
#    使用 file(GLOB_RECURSE) 来自动发现 client/src/ 目录下的所有 .cpp 文件。
#    这样以后你增加或删除客户端的源文件时，就不需要手动修改这个 CMake 文件了。
file(GLOB_RECURSE CLIENT_SOURCES "src/*.cpp")

# --- 3. 定义可执行文件目标 ---
#    创建一个名为 "client" 的可执行文件。
add_executable(client ${CLIENT_SOURCES})

# --- 4. 链接所有依赖 ---
#    这是最核心的一步。它告诉 CMake 如何将我们的代码与所需的库组合在一起。
target_link_libraries(client PRIVATE
    # 1. 链接我们自己定义的 common 库。
    #    CMake 会自动处理 common 库的包含目录 (`common/src`, `build/common`)
    #    以及 common 库自身的依赖 (protobuf)。
    #    这就是模块化的巨大优势！
    common

    # 2. 链接第三方库。
    asio::asio
    
)
