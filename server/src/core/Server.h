#pragma once

#include <iostream>
#include <asio.hpp>
#include <memory>

class Session;
class SessionManager;
class AuthService;
class RoomService;
class MessageService;
class IUserRepository;
class IRoomRepository;
class IMessageRepository;
namespace chat{
    class Envelope;
}
class Server{
public:
       Server(asio::io_context& io_context,unsigned short port);
       Server(const Server&) = delete; 
       Server& operator=(const Server&) = delete;
       ~Server(); 
       void run();
       void onMessage(std::shared_ptr<Session> session, const chat::Envelope& envelope);
       void onDisconnect(std::shared_ptr<Session> session);
private:
       asio::ip::tcp::acceptor acceptor;
       asio::io_context& ioc;

       std::unique_ptr<IUserRepository> userRepository;
       std::unique_ptr<IRoomRepository> roomRepository;
       std::unique_ptr<IMessageRepository> messageRepository;
       std::unique_ptr<SessionManager> sessionManager;
       std::unique_ptr<AuthService> authService;
       std::unique_ptr<RoomService> roomService;
       std::unique_ptr<MessageService> messageService;

       void start_accept();
       void handle_accept(const asio::error_code& ec, std::shared_ptr<Session> session);
       void dispatchMessage(std::shared_ptr<Session> session, const chat::Envelope& envelope);
};