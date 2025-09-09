// client/src/main.cpp
#include "client.h"
#include <iostream>
#include <string>
#include <thread>
#include <regex>

std::unique_ptr<asio::io_context> io_context;
std::shared_ptr<Client> client;

void signal_handler(int signum) {
    std::cout << "\n[System] Interrupt signal (" << signum << ") received. Shutting down...\n";
    // 通过 post 将关闭操作调度到 io_context 线程，保证线程安全
    if (client && io_context) {
        asio::post(*io_context, []() {
            if (client) {
                client->close();
            }
        });
    }
}
void print_help() {
    std::cout << "--- Chat Client Commands ---\n"
        << "/register <username> <password>\n"
        << "/login <username> <password>\n"
        << "/create <room_name>\n"
        << "/join <room_name>\n"
        << "/leave\n"
        << "/quit\n"
        << "@<username> <message>  (to send a private message)\n"
        << "any other text for public message in the current room.\n"
        << "---------------------------\n";
}
void handle_user_input() {
    print_help();
    std::string line;
    while (std::getline(std::cin, line)) {
        if (!client) break; // 如果客户端已关闭，退出循环
        if (line.empty()) continue;
        if (line == "/quit") {
            // 用户请求退出
            client->close();
            break; // 退出输入循环
        }
        Envelope envelope;
        bool should_send = true;
        if (line.rfind("/register ", 0) == 0) {
            std::stringstream ss(line);
            std::string cmd, user, pass;
            ss >> cmd >> user >> pass;
            if (user.empty() || pass.empty()) {
                std::cout << "[System] Usage: /register <username> <password>" << std::endl;
                should_send = false;
            }
            else {
                auto* req = envelope.mutable_registration_request();
                req->set_username(user);
                req->set_password(pass);
            }
        }
        else if (line.rfind("/login ", 0) == 0) {
            std::stringstream ss(line);
            std::string cmd, user, pass;
            ss >> cmd >> user >> pass;
            envelope.mutable_login_request()->set_username(user);
            envelope.mutable_login_request()->set_password(pass);
        }
        else if (line.rfind("/create ", 0) == 0) {
            std::string roomName = line.substr(8);
            auto* req = envelope.mutable_room_operation_request();
            req->set_operation(chat::RoomOperation::CREATE);
            req->set_room_name(roomName);
        }
        else if (line.rfind("/join ", 0) == 0) {
            std::string roomName = line.substr(6);
            auto* req = envelope.mutable_room_operation_request();
            req->set_operation(chat::RoomOperation::JOIN);
            req->set_room_name(roomName);
        }
        else if (line == "/leave") {
            std::string currentRoom = client->getCurrentRoom();
            if (currentRoom.empty()) {
                std::cout << "[System] You are not in any room to leave." << std::endl;
                should_send = false;
            }
            else {
                auto* req = envelope.mutable_room_operation_request();
                req->set_operation(chat::RoomOperation::LEAVE);
                req->set_room_name(currentRoom);
            }
        }
        else {
            static const std::regex pm_pattern("^@(\\S+)\\s+(.+)");
            std::smatch match;
            if (std::regex_match(line, match, pm_pattern)) {
                auto* req = envelope.mutable_private_message_request();
                req->set_to_username(match[1].str());
                req->set_content(match[2].str());
            }
            else {
                envelope.mutable_public_message()->set_content(line);
            }
        }
        if (should_send) {
            client->send(envelope);
        }
    }
    std::cout << "[System] Input thread finished." << std::endl;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: chat_client <host> <port>\n";
        return 1;
    }
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    try {
        // --- 初始化全局对象 ---
        io_context = std::make_unique<asio::io_context>();
        client = std::make_shared<Client>(*io_context);

        // --- 异步连接 ---
        client->connect(argv[1], std::stoi(argv[2]),
            [](const asio::error_code& ec) {
                if (!ec) {
                    std::cout << "[System] Connection established. You can start chatting.\n";
                }
                else {
                    std::cerr << "[System] Connection failed: " << ec.message() << "\n";
                    // 连接失败，停止 io_context 以便程序退出
                    if (client) client->close();
                }
            });
        // --- 启动输入线程 ---
        std::thread input_thread(handle_user_input);
        // --- 主线程驱动 I/O ---
        // run() 会一直阻塞，直到 work_guard 被 reset
        io_context->run();
        // --- 清理 ---
        // 当 run() 返回后，意味着关闭流程已启动
        // 我们需要确保输入线程也退出
        // 在 Windows 上，可能需要模拟一次回车来唤醒 getline
        // (这是一个复杂的控制台 I/O 问题，暂时可以忽略)
        if (input_thread.joinable()) {
            // Detach 是一个简单的处理方式，让它自生自灭
            // 更好的方式是使用异步 I/O 来处理 stdin
            input_thread.detach();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Main exception: " << e.what() << std::endl;
    }

    std::cout << "[System] Client has shut down." << std::endl;
    return 0;
}