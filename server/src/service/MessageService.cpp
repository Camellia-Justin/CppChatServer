#include "MessageService.h"
void MessageService::handlePublicMessage(std::shared_ptr<Session> session, const chat::PublicMessage& publicMessage) {
    chat::Envelope response;
    if (!session || !session->isAuthenticated()) {
        std::cerr << "Warning: Unauthenticated session tried to send a public message." << std::endl;
        return;
    }
    const int senderId = session->getUserId();
    const std::string roomName = roomService->getUserCurrentRoomName(senderId);

    if (roomName.empty()) {
        std::cerr << "Warning: User " << session->getUsername() 
                  << " (ID: " << senderId 
                  << ") tried to send a message without being in a room." << std::endl;
        auto* error_response = response.mutable_error_response();
        error_response->set_error_message("You are not in any room. Join a room to send messages.");
        error_response->set_error_code(403);
        session->send(response);
        return;
    }
    Message message;
    message.setSenderId(senderId);
    message.setRoomId(roomService->getUserCurrentRoomId(senderId));
    message.setContent(publicMessage.content());
    messageRepository->addMessage(message);

    auto* messageBroadcast = response.mutable_message_broadcast();
    messageBroadcast->set_from_user_id(std::to_string(senderId));
    messageBroadcast->set_from_username(session->getUsername());
    messageBroadcast->set_content(publicMessage.content());
    messageBroadcast->set_room_name(roomName);
    *(messageBroadcast->mutable_timestamp()) = google::protobuf::util::TimeUtil::GetCurrentTime();

    roomService->broadcastToRoom(roomService->getUserCurrentRoomName(session->getUserId()), response);
}

void MessageService::handlePrivateMessage(std::shared_ptr<Session> session, const chat::PrivateMessageRequest& privateMessage) {
    chat::Envelope response;
    if (!session || !session->isAuthenticated()) {
        std::cerr << "Warning: Unauthenticated session tried to send a private message." << std::endl;
        return;
    }
    long long senderId = session->getUserId();
    std::string senderName = session->getUsername();
    std::string roomname = roomService->getUserCurrentRoomName(senderId);

    if (roomname.empty()) {
        std::cerr << "Warning: User " << session->getUsername() 
                  << " (ID: " << senderId 
                  << ") tried to send a private message without being in a room." << std::endl;
        auto* error_response = response.mutable_error_response();
        error_response->set_error_message("You are not in any room. Join a room to send messages.");
        error_response->set_error_code(403);
        session->send(response);
        return;
    }

    auto acceptSession = sessionManager->findByUsername(privateMessage.to_username());
    if (!acceptSession || !acceptSession->isAuthenticated()) {
        std::cerr << "Warning: User " << session->getUsername() 
                  << " (ID: " << senderId 
                  << ") tried to send a private message to non-existent or unauthenticated user: " 
                  << privateMessage.to_username() << std::endl;
        auto* error_response = response.mutable_error_response();
        error_response->set_error_message("The user you are trying to message does not exist or is not online.");
        error_response->set_error_code(404);
        session->send(response);
        return;
    }
    if (acceptSession->getUserId() == senderId) {
        std::cerr << "Warning: User " << session->getUsername() 
                  << " (ID: " << senderId 
                  << ") tried to send a private message to themselves." << std::endl;
        auto* error_response = response.mutable_error_response();
        error_response->set_error_message("You cannot send a private message to yourself.");
        error_response->set_error_code(400);
        session->send(response);
        return;
    }
    Message message;
    message.setSenderId(senderId);
    message.setRoomId(roomService->getUserCurrentRoomId(senderId));
    message.setContent(privateMessage.content());
    messageRepository->addMessage(message);
    auto* messageBroadcast = response.mutable_message_broadcast();
    messageBroadcast->set_from_user_id(std::to_string(senderId));
    messageBroadcast->set_from_username(senderName);
    messageBroadcast->set_content(privateMessage.content());
    messageBroadcast->set_room_name(roomname);
    *(messageBroadcast->mutable_timestamp()) = google::protobuf::util::TimeUtil::GetCurrentTime();
    acceptSession->send(response);
    session->send(response);
}
