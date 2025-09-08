// client/src/main.cpp
#include "Client.h"
#include <iostream>
#include <string>
#include <thread>
#include <regex>

void print_help() {
    std::cout << "--- Chat Client Commands ---\n"
              << "/login <username> <password>\n"
              << "/join <room_name>\n"
              << "/leave\n"
              << "/quit\n"
              << "@<username> <message>  (to send a private message)\n"
              << "any other text for public message in the current room.\n"
              << "---------------------------\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: chat_client <host> <port>\n";
        return 1;
    }

    try {
        asio::io_context io_context;
        auto client = std::make_shared<Client>(io_context);

        client->connect(argv[1], std::stoi(argv[2]));

        std::thread t([&io_context]() { 
            try {
                io_context.run(); 
            } catch (const std::exception& e) {
                std::cerr << "IO context exception: " << e.what() << std::endl;
            }
        });

        print_help();
        
        // --- 主线程的用户输入循环 ---
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.empty()) continue;

            if (line == "/quit") {
                break;
            }

            Envelope envelope;
            bool should_send = true;

            // --- 解析用户输入 ---
            if (line.rfind("/login ", 0) == 0) {
                std::stringstream ss(line);
                std::string cmd, user, pass;
                ss >> cmd >> user >> pass;
                envelope.mutable_login_request()->set_username(user);
                // envelope.mutable_login_request()->set_password(pass);
            } else if (line.rfind("/join ", 0) == 0) {
                std::string roomName = line.substr(6);
                auto* req = envelope.mutable_room_operation_request();
                req->set_operation(chat::RoomOperation::JOIN);
                req->set_room_name(roomName);
            } else if (line == "/leave") {
                auto* req = envelope.mutable_room_operation_request();
                req->set_operation(chat::RoomOperation::LEAVE);
            }
            else {
                // 使用我们之前讨论的正则来区分公聊和私聊
                static const std::regex pm_pattern("^@(\\S+)\\s+(.+)");
                std::smatch match;
                if (std::regex_match(line, match, pm_pattern)) {
                    auto* req = envelope.mutable_private_message_request();
                    req->set_to_username(match[1].str());
                    req->set_content(match[2].str());
                } else {
                    envelope.mutable_public_message()->set_content(line);
                }
            }

            if (should_send) {
                client->send(envelope);
            }
        }
        client->close(); 
        if (t.joinable()) {
            t.join(); 
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}