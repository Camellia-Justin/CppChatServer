#pragma once

#include "chat.pb.h"
#include "session/Session.h"

#include <google/protobuf/util/time_util.h>
#include <util/TimeConvert.h>
#include "core/SessionManager.h"
#include "data/DataAccess.h"

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif
class RoomService {
private:
    struct ActiveRoom {
        long long id; 
        long long creator_id;
        std::string name;
        std::unordered_map<long long, std::shared_ptr<Session>> members;
    };
public:
    RoomService(IRoomRepository* roomRepository,IUserRepository* userRepository, IMessageRepository* messageRepository, SessionManager* sessionManager) : roomRepository(roomRepository), userRepository(userRepository), messageRepository(messageRepository), sessionManager(sessionManager) {}
    void handleRoomOperation(std::shared_ptr<Session> session, const chat::RoomOperationRequest& request);
    void handleHistoryRequest(std::shared_ptr<Session> session, const chat::HistoryMessageRequest& request);
    void handleDisconnect(std::shared_ptr<Session> session);
    std::string getUserCurrentRoomName(long long userId);
    long long getUserCurrentRoomId(long long userId);
    void broadcastToRoom(const std::string& roomName, const chat::Envelope& envelope, long long excludeUserId = 0);
private:
    IRoomRepository* roomRepository;
    IMessageRepository* messageRepository;
    IUserRepository* userRepository;
    SessionManager* sessionManager;
    mutable std::mutex mutex;
    std::unordered_map<std::string, ActiveRoom> activeRooms;//roomname,ActiveRoom
    std::unordered_map<long long, std::string> userToRoomMap;//userid,roomname
};