#pragma once

#include <string>
#include <chrono>
class Message{
public:
    long long getId() const { return id; }
    void setId(long long newId) { id = newId; }

    long long getSenderId() const { return sender_id; }
    void setSenderId(long long newSenderId) { sender_id = newSenderId; }

    long long getRoomId() const { return room_id; }
    void setRoomId(long long newRoomId) { room_id = newRoomId; }

    std::string getContent() const { return content; }
    void setContent(const std::string& newContent) { content = newContent; }

    std::chrono::system_clock::time_point getCreatedAt() const { return created_at; }
    void setCreatedAt(const std::chrono::system_clock::time_point& newCreatedAt) { created_at = newCreatedAt; }
private:
    long long id;
    long long room_id;
    long long sender_id;
    std::string content;
    std::chrono::system_clock::time_point created_at;
};