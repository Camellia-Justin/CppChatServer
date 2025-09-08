#pragma once

#include "IMessageRepository.h"

class MySQLMessageRepository : public IMessageRepository {
public:
    MySQLMessageRepository() = default;
    ~MySQLMessageRepository() = default;

    std::optional<Message> findByMessageId(long long id);
    std::vector<Message> findBySenderId(long long sender_id);
    std::vector<Message> findByRoomId(long long room_id);
    std::vector<Message> findByContent(const std::string& content);
    std::vector<Message> findLatestByRoomId(long long roomId, int limit);
    bool addMessage(Message& message);
    bool removeMessage(long long id);
};
