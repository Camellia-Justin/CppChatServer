#pragma once

#include "data/IMessageRepository.h"
#include <google/protobuf/util/time_util.h>
#include <chrono>
#include "chat.pb.h"
#include "core/SessionManager.h"
#include "session/Session.h"
#include "RoomService.h"
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif
class MessageService {
public:
    MessageService(IMessageRepository* messageRepository, SessionManager* sessionManager, RoomService* roomService) : messageRepository(messageRepository), sessionManager(sessionManager), roomService(roomService) {}
    void  handlePublicMessage(std::shared_ptr<Session> session, const chat::PublicMessage& publicMessage);
    void  handlePrivateMessage(std::shared_ptr<Session> session, const chat::PrivateMessageRequest& privateMessage);
private:
    IMessageRepository* messageRepository;
    RoomService* roomService;
    SessionManager* sessionManager;
};