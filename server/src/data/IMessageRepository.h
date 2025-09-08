#pragma once

#include <vector>
#include <optional>
#include "domain/Message.h"

class IMessageRepository {
public:
    virtual ~IMessageRepository() = default;

    virtual std::optional<Message> findByMessageId(long long id) = 0;
    virtual std::vector<Message> findBySenderId(long long sender_id) = 0;
    virtual std::vector<Message> findByRoomId(long long room_id) = 0;
    virtual std::vector<Message> findByContent(const std::string& content) = 0;
    virtual std::vector<Message> findLatestByRoomId(long long roomId, int limit) = 0;
    virtual bool addMessage(Message& message) = 0;
    virtual bool removeMessage(long long id) = 0;
};
