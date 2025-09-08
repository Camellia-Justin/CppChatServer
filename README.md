# C++ 高并发多房间聊天服务器 (ChatServer)

这是一个基于 C++17 与 Boost.Asio 构建的高性能、跨平台、多房间实时聊天服务器项目。项目采用现代 C++ 设计原则，实现了清晰的分层架构，支持多用户并发通信、用户认证、房间管理、公共聊天与私聊等核心功能。

## 核心特性

*   **高并发网络**: 基于 **Asio** 的 Proactor 异步 I/O 模型，支持大量并发连接。
*   **跨平台**: 使用 **CMake** 和 **vcpkg** 构建，可在 Linux 和 Windows 等主流平台下无缝编译和运行。
*   **模块化架构**: 严格遵循**三层架构**（网络层、服务层、数据访问层）和**依赖注入**原则，实现了高度解耦。
*   **安全认证**: 用户密码采用**加盐哈希** 存储，保证账户安全。
*   **实时通信**:
    *   支持多**聊天室 (Rooms)**，消息在房间内广播。
    *   支持**房间内私聊**功能。
    *   实时**用户状态通知**（加入/离开房间）。
*   **数据持久化**:
    *   使用 **MySQL/MariaDB** 数据库存储用户信息、房间信息和聊天记录。
    *   通过**数据库连接池**高效管理数据库连接，避免性能瓶颈。
*   **健壮的协议**:
    *   使用 **Protobuf** 定义清晰、高效且向后兼容的客户端-服务器通信协议。
    *   自定义应用层协议，通过**“长度-内容”**格式解决了 TCP **粘包**问题。

## 技术栈

| 类别 | 技术 |
| :--- | :--- |
| **语言** | C++17 |
| **网络库** | Asio |
| **序列化** | Protobuf |
| **数据库** | MySQL/MariaDB, SOCI (C++ 数据库访问库) |
| **构建系统** | CMake |
| **依赖管理** | vcpkg |
| **配置文件** | nlohmann/json |

## 架构设计

本项目采用经典的**分层架构**，将不同职责的模块清晰地分离，并通过**面向接口编程**来降低耦合度。

*   **Common (共享层)**: 存放客户端与服务器共享的代码，包括 Protobuf 协议定义和数据库实体类 (`domain`)。
*   **Server (服务器)**:
    *   **Network (网络层)**: 封装 `session`，处理底层异步 I/O、粘包和协议解析。
    *   **Data (数据访问层)**: 通过**仓库模式 (Repository Pattern)** 封装所有数据库操作，使用**连接池**管理连接。
    *   **Service (服务层)**: 实现所有核心业务逻辑，如认证、房间管理、消息转发。
    *   **Core (核心层)**: 包含 `Server` 主类和 `SessionManager`，负责组装和协调所有模块。
*   **Client (客户端)**: 一个多线程的命令行客户端，用于演示和测试服务器功能。

## 构建与运行

### 1. 前置依赖

*   [Git](https://git-scm.com/)
*   [CMake](https://cmake.org/download/) (版本 >= 3.15)
*   C++ 编译器 (GCC, Clang, MSVC)
*   [vcpkg](https://github.com/microsoft/vcpkg)

### 2. 设置 vcpkg

请确保 `vcpkg` 已经安装

### 3. 安装项目依赖

在 vcpkg 根目录下运行以下命令，安装本项目所需的所有第三方库。

```bash
# 对于 Linux
./vcpkg install asio protobuf soci[mysql] nlohmann-json --triplet x64-linux

# 对于 Windows
.\vcpkg.exe install asio protobuf soci[mysql] nlohmann-json --triplet x64-windows
```

### 4. 数据库设置

本项目使用 MySQL/MariaDB。请先创建数据库，然后导入表结构。

```bash
# 1. 登录数据库
mysql -u root -p

# 2. 创建数据库
CREATE DATABASE chat_server_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
EXIT;

# 3. 从项目根目录导入表结构
mysql -u root -p chat_server_db < schema.sql
```

### 5. 修改配置文件

将项目根目录下的 `config.example.json` 复制为 `config.json`，并填入你自己的数据库密码。

```bash
cp config.example.json config.json
# 然后用文本编辑器修改 config.json
```

### 6. 编译项目

```bash
# 1. 创建构建目录
mkdir build
cd build

# 2. 运行 CMake 进行配置
#    在 Linux 上，vcpkg 工具链通常会被自动找到
cmake ..

#    在 Windows 上，你可能需要手动指定
#    cmake .. -DCMAKE_TOOLCHAIN_FILE C:/path/to/your/vcpkg/scripts/buildsystems/vcpkg.cmake

# 3. 编译
cmake --build .
```

### 7. 运行

编译成功后，可执行文件会生成在项目根目录的 `bin/` 文件夹下。

**启动服务器**:
打开一个终端，运行：
```bash
./bin/server
```

**启动客户端**:
打开**另一个**终端，运行 (IP 地址和端口应与 `config.json` 匹配):
```bash
./bin/client 127.0.0.1 12345
```
现在你可以打开多个客户端实例进行聊天了！

