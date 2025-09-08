#include "chat.pb.h"
#include "session/Session.h"
#include "core/SessionManager.h"
#include "data/MySQLUserRepository.h"
#include "data/MySQLRoomRepository.h"
#include "data/MySQLMessageRepository.h"
#include "service/AuthService.h"
#include "service/RoomService.h"
#include "service/MessageService.h"
#include "Server.h"
#include <iostream>

Server::Server(asio::io_context& io_context,unsigned short port)
:ioc(io_context),
 acceptor(std::make_shared<asio::ip::tcp::acceptor>(io_context,asio::ip::tcp::endpoint(asio::ip::tcp::v4(),port))){
    
    userRepository = std::make_unique<MySQLUserRepository>();
    roomRepository = std::make_unique<MySQLRoomRepository>();
    messageRepository = std::make_unique<MySQLMessageRepository>();

    sessionManager = std::make_unique<SessionManager>();
    authService = std::make_unique<AuthService>(userRepository.get(), sessionManager.get());
    roomService = std::make_unique<RoomService>(roomRepository.get(), userRepository.get(), messageRepository.get(), sessionManager.get());
    messageService = std::make_unique<MessageService>(messageRepository.get(), sessionManager.get(), roomService.get());
}
Server::~Server() = default;
void Server::run(){
    start_accept();
}
void Server::start_accept(){
    std::shared_ptr<asio::ip::tcp::socket> sock_ptr = std::make_shared<asio::ip::tcp::socket>(ioc);
    acceptor->async_accept(*sock_ptr,
        [this,sock_ptr](const asio::error_code& ec){
            handle_accept(ec,std::make_shared<Session>(sock_ptr,*this));
        });
}
void Server::handle_accept(const asio::error_code& ec, std::shared_ptr<Session> session){
    if(!ec){
        try{
            std::cout<<"New connection from "<<session->socket_ptr->remote_endpoint().address().to_string()<<":"<<session->socket_ptr->remote_endpoint().port()<<std::endl; 
        session->start();
        }catch(const std::exception& e){
            std::cerr<<"Exception in starting session: "<<e.what()<<std::endl;
        }
    }else {
    std::cout << "Error accepting connection: " << ec.message() << std::endl;
}
    start_accept();
}
void Server::onMessage(std::shared_ptr<Session> session, const chat::Envelope& envelope){
    if (session->isAuthenticated()) {
        std::cout << "Received message from user '" << session->getUsername() 
                  << "' (ID: " << session->getUserId() << "). "
                  << "Payload type: " << envelope.payload_case() << std::endl;
    } else {
        std::cout << "Received message from an unauthenticated session. "
                  << "Payload type: " << envelope.payload_case() << std::endl;
    }
    asio::post(ioc, [this, session, envelope]() {
        dispatchMessage(session, envelope);
    });
}
void Server::dispatchMessage(std::shared_ptr<Session> session, const chat::Envelope& envelope){
    switch(envelope.payload_case()){
        case chat::Envelope::kLoginRequest:
            authService->handleLogin(session,envelope.login_request());
            return;
        case chat::Envelope::kRegistrationRequest:
            authService->handleRegister(session,envelope.registration_request());
            return;
        default:
            return;
    }
    if (!session->isAuthenticated()) {
        std::cerr << "Warning: Unauthenticated session tried to send an authenticated-only request. Payload type: "
                  << envelope.payload_case() << std::endl;
        chat::Envelope response_envelope;
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("Authentication required.");
        session->send(response_envelope);
        return;
    }
    switch (envelope.payload_case())
    {
        case chat::Envelope::kChangePasswordRequest:
            authService->handleChangePassword(session,envelope.change_password_request());
            break;
        case chat::Envelope::kChangeUsernameRequest:
            authService->handleChangeUsername(session,envelope.change_username_request());
            break;
        case chat::Envelope::kPublicMessage:
            messageService->handlePublicMessage(session,envelope.public_message());
            break;
        case chat::Envelope::kPrivateMessageRequest:
            messageService->handlePrivateMessage(session,envelope.private_message_request());
            break;
        case chat::Envelope::kRoomOperationRequest:
            roomService->handleRoomOperation(session,envelope.room_operation_request());
            break;
        case chat::Envelope::kHistoryMessageRequest:
            roomService->handleHistoryRequest(session,envelope.history_message_request());
            break;
        default:
            std::cerr << "Warning: Received Envelope with unknown payload type: " 
                      << envelope.payload_case() << std::endl;
            break;
    }
}
void Server::onDisconnect(std::shared_ptr<Session> session){
    asio::post(ioc, [this, session]() {
        if (session->isAuthenticated()) {
            std::cout << "User '" << session->getUsername() 
                      << "' (ID: " << session->getUserId() << ") disconnected." << std::endl;
            roomService->handleDisconnect(session);
        } else {
            std::cout << "An anonymous connection closed." << std::endl;
        }
        sessionManager->remove(session);
    });
}

