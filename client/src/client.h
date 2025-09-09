// client/src/ChatClient.h
#pragma once

#include <asio.hpp>
#include <memory>
#include <string>
#include <deque>
#include <thread>
#include <asio/executor_work_guard.hpp>
#include "chat.pb.h"

using Envelope = chat::Envelope;

class Client : public std::enable_shared_from_this<Client> {
public:
    Client(asio::io_context& io_context);
    void connect(const std::string& host, unsigned short port);
    void close();
    void send(const Envelope& envelope);
    std::string getCurrentRoom() const { return currentRoom; }
    void setCurrentRoom(const std::string& roomName) { currentRoom = roomName; }
private:
    void do_read_header();
    void do_read_body(uint32_t body_length);
    void handle_server_message(const Envelope& envelope); // 处理收到的消息

    void do_write();
    void handle_write(const asio::error_code& ec, size_t bytes_transferred);

    void handle_error(const std::string& where, const asio::error_code& ec);

    asio::io_context& io_context;
    asio::ip::tcp::socket socket;
    asio::executor_work_guard<asio::io_context::executor_type> work_guard;
    
    std::array<char, 4> read_header;
    std::vector<char> read_body;
    
    std::deque<std::string> write_queue;
    uint32_t header = 4;
    static const uint32_t max_body_length = 8192;

    std::string currentRoom;
};