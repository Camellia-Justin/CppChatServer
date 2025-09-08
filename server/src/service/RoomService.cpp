#include "RoomService.h"
    void RoomService::handleRoomOperation(std::shared_ptr<Session> session, const chat::RoomOperationRequest& request){
        chat::Envelope response;
        if (!session || !session->isAuthenticated()) {
            std::cerr << "Warning: Unauthenticated session tried to perform room operation." << std::endl;
            return;
        }
        long long userId = session->getUserId();
        std::string roomname = request.room_name();
        switch (request.operation()){
            case 0://join
            {
                {
                    std::lock_guard lock(mutex);
                    auto userIt = userToRoomMap.find(userId);
                    if (userIt != userToRoomMap.end()) {
                        const std::string& oldRoomName = userIt->second;
                        auto roomIt = activeRooms.find(oldRoomName);
                        if (roomIt != activeRooms.end()) {
                            roomIt->second.members.erase(userId);
                            if (roomIt->second.members.empty()) {
                                activeRooms.erase(roomIt);
                            }
                        }
                        userToRoomMap.erase(userIt);
                    }
                    userToRoomMap[userId]=roomname;
                    activeRooms[roomname].members[userId]=session;
                }
                response.mutable_room_operation_response()->set_success(true);
                response.mutable_room_operation_response()->set_message("Joined room "+roomname+" successfully.");
                chat::Envelope joinNotification;
                auto* notification = joinNotification.mutable_server_notification();
                notification->set_event_type(chat::UserEventType::USER_JOINED);
                notification->set_user_id(std::to_string(userId));
                notification->set_username(session->getUsername());
                notification->set_message("User "+session->getUsername()+" has joined the room.");
                broadcastToRoom(roomname, joinNotification, userId);
                break;
            }
            case 1://left
            {
                {
                    std::lock_guard lock(mutex);
                    auto userIt = userToRoomMap.find(userId);
                    if (userIt != userToRoomMap.end() && userIt->second == roomname) {
                        userToRoomMap.erase(userIt);
                        auto roomIt = activeRooms.find(roomname);
                        if (roomIt != activeRooms.end()) {
                            roomIt->second.members.erase(userId);
                            if (roomIt->second.members.empty()) {
                                activeRooms.erase(roomIt);
                            }
                        }
                    }
                }
                response.mutable_room_operation_response()->set_success(true);
                response.mutable_room_operation_response()->set_message("Left room "+roomname+" successfully.");
                chat::Envelope leaveNotification;
                auto* notification = leaveNotification.mutable_server_notification();
                notification->set_event_type(chat::UserEventType::USER_LEFT);
                notification->set_user_id(std::to_string(userId));
                notification->set_username(session->getUsername());
                notification->set_message("User "+session->getUsername()+" has left the room.");
                broadcastToRoom(roomname, leaveNotification, userId);
                break;
            }
            case 2://create
            {
                if (roomRepository->findByRoomName(roomname)) {
                    response.mutable_room_operation_response()->set_success(false);
                    response.mutable_room_operation_response()->set_message("Room name '" + roomname + "' is already taken.");
                    break;
                }
                Room room;
                room.setName(roomname);
                room.setCreatorId(userId);
                roomRepository->addRoom(room);
                {
                    std::lock_guard lock(mutex);
                    userToRoomMap[userId]=roomname;
                    activeRooms[roomname].id = room.getId();
                    activeRooms[roomname].creator_id = userId;
                    activeRooms[roomname].name = room.getName();
                    activeRooms[roomname].members[userId]=session;
                }
                response.mutable_room_operation_response()->set_success(true);
                response.mutable_room_operation_response()->set_message("Created room "+roomname+" successfully.");
                break;
            }
            default:
                response.mutable_room_operation_response()->set_success(false);
                response.mutable_room_operation_response()->set_message("Unknown operation.");
                break;
        }
        response.mutable_room_operation_response()->set_operation(request.operation());
        response.mutable_room_operation_response()->set_room_name(roomname);
        session->send(response);
    }
    void RoomService::handleHistoryRequest(std::shared_ptr<Session> session, const chat::HistoryMessageRequest& request){
        chat::Envelope response;
        if (!session || !session->isAuthenticated()) {
            std::cerr << "Warning: Unauthenticated session tried to request message history." << std::endl;
            return;
        }
        std::string roomname = request.room_name();
        {
            std::lock_guard lock(mutex);
            auto userIt = userToRoomMap.find(session->getUserId());
            if (userIt == userToRoomMap.end() || userIt->second != roomname) {
                std::cerr << "Warning: User tried to request message history for a room they are not part of." << std::endl;
                return;
            }
        }
        long long roomId = getUserCurrentRoomId(session->getUserId());
        int limit = request.limit() > 0 ? request.limit() : 50;
        std::vector<Message> messages(std::move(messageRepository->findLatestByRoomId(roomId, limit)));
        for(const auto& msg:messages){
            auto* historyMsg = response.mutable_history_message_response()->add_messages();
            historyMsg->set_from_user_id(std::to_string(msg.getSenderId()));
            historyMsg->set_from_username(userRepository->findByUserId(msg.getSenderId())->getUsername());
            historyMsg->set_content(msg.getContent());
            historyMsg->set_room_name(roomname);
            convertTimePointToTimestamp(msg.getCreatedAt(), historyMsg->mutable_timestamp());
        }
        response.mutable_history_message_response()->set_room_name(roomname);
        session->send(response);
    }
    void RoomService::handleDisconnect(std::shared_ptr<Session> session){
        if (!session || !session->isAuthenticated()) {
            return;
        }
        long long userId = session->getUserId();
        std::lock_guard lock(mutex);
        auto userIt = userToRoomMap.find(userId);
        if (userIt != userToRoomMap.end()) {
            const std::string& roomName = userIt->second;
            
            chat::Envelope leaveNotification;
            auto* notification = leaveNotification.mutable_server_notification();
            notification->set_event_type(chat::UserEventType::USER_LEFT);
            notification->set_user_id(std::to_string(userId));
            notification->set_username(session->getUsername());
            notification->set_message("User "+session->getUsername()+" has left the room.");
            broadcastToRoom(roomName, leaveNotification, userId);

            auto roomIt = activeRooms.find(roomName);
            if (roomIt != activeRooms.end()) {
                roomIt->second.members.erase(userId);
                if (roomIt->second.members.empty()) {
                    activeRooms.erase(roomIt);
                }
            }
            userToRoomMap.erase(userIt);
        }
    }
    std::string RoomService::getUserCurrentRoomName(int userId){
        std::lock_guard lock(mutex);
        auto it = userToRoomMap.find(userId);
        if (it != userToRoomMap.end()) {
            return it->second;
        }
        return "";
    }
    long long RoomService::getUserCurrentRoomId(int userId){
        std::lock_guard lock(mutex);
        auto it = userToRoomMap.find(userId);
        if (it != userToRoomMap.end()) {
            const std::string& roomName = it->second;
            auto roomIt = activeRooms.find(roomName);
            if (roomIt != activeRooms.end()) {
                return roomIt->second.id;
            }
        }
        return 0;
    }
    void RoomService::broadcastToRoom(const std::string& roomName, const chat::Envelope& envelope, int excludeUserId) {
        std::vector<std::shared_ptr<Session>> recipients;
        { 
            std::lock_guard<std::mutex> lock(mutex);
            auto roomIt = activeRooms.find(roomName);
            if (roomIt == activeRooms.end()) {
                return;
            }
            const ActiveRoom& room = roomIt->second;
            recipients.reserve(room.members.size());
            for (const auto& memberPair : room.members) {
                if (memberPair.first != excludeUserId) {
                    recipients.push_back(memberPair.second);
                }
            }
        }
        for (const auto& session_ptr : recipients) {
            if (session_ptr) {
                session_ptr->send(envelope);
            }
        }
    }