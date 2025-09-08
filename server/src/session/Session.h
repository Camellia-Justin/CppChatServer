#pragma once

#include <iostream>
#include <asio.hpp>
#include <memory>
#include <queue>
#include <optional>
#include "chat.pb.h"
class Server;
class Session:public std::enable_shared_from_this<Session>{
public:
    Session(std::shared_ptr<asio::ip::tcp::socket> sock,Server& srv);
    ~Session()=default;
    void start();
    void send(const chat::Envelope& envelope);
    void setAuthenticated(long long userId, const std::string& username);
    bool isAuthenticated() const;
    void clearAuthentication();
    long long getUserId() const;
    std::string getUsername() const;
    void setUsername(const std::string& newUsername);
    
private:
    friend class Server;
    void do_read_header();
    void do_read_body(uint32_t body_length);
    void do_write();
    void handle_write(const asio::error_code& ec,size_t bytes_transferred);
    void handle_error(const std::string& what,const asio::error_code& ec);

    Server& server;
    bool is_closed = false;
    std::shared_ptr<asio::ip::tcp::socket> socket_ptr;
    std::queue<std::string> message_queue;
    static constexpr size_t header_length = 4;
    std::array<char,header_length> header_buf;
    std::vector<char> body_buf;
    static const uint32_t max_body_length = 8192;

    std::optional<long long> userId;
    std::optional<std::string> username;
    bool Authenticated = false;
};