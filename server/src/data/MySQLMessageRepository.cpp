#include "MySQLMessageRepository.h"
#include "ConnectionPool.h"
#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>
#include <iostream>
#include <set>

template<>
struct soci::type_conversion<Message>{
    static void from_base(soci::values& v, indicator,Message& msg){
        msg.setId(v.get<long long>("id"));
        msg.setRoomId(v.get<long long>("room_id"));
        msg.setSenderId(v.get<long long>("sender_id"));
        msg.setContent(v.get<std::string>("content"));
        msg.setCreatedAt(v.get<std::chrono::system_clock::time_point>("created_at"));
    }
    static void to_base(const Message& msg, soci::values& v, soci::indicator& ind){
        v.set("room_id", msg.getRoomId());
        v.set("sender_id", msg.getSenderId());
        v.set("content", msg.getContent());
        ind = soci::i_ok;
    }
};
bool MySQLMessageRepository::addMessage(Message& msg){
    try{
        auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
        sql << "INSERT INTO messages (room_id, sender_id, content) VALUES (:room_id, :sender_id, :content)",
            soci::use(msg);
         long long newId;
        if (!sql.get_last_insert_id("messages", newId)) {
            std::cerr << "Failed to get last insert ID for messages." << std::endl;
            return false;
        }
        msg.setId(newId);
        try{
        std::chrono::system_clock::time_point createdAt;
        sql << "SELECT created_at FROM messages WHERE id = :id", soci::use(newId), soci::into(createdAt);
        msg.setCreatedAt(createdAt);
        }catch(const std::exception& e){ 
            std::cerr << "Error retrieving created_at: " << e.what() << std::endl;
            return false;
        }
        tr.commit();
        return true;
    }catch(const std::exception& e){
        std::cerr << "Error adding message: " << e.what() << std::endl;
        return false;
    }
}
bool MySQLMessageRepository::removeMessage(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        sql<<"DELETE FROM messages WHERE id = :id", soci::use(id);
        tr.commit();
        return true;

    }catch(const std::exception& e){
        std::cerr << "Error removing message: " << e.what() << std::endl;
        return false;
    }
}
std::optional<Message> MySQLMessageRepository::findByMessageId(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        Message msg;
        sql<<"SELECT id, room_id, sender_id, content, created_at FROM messages WHERE id = :id", soci::use(id), soci::into(msg);
        tr.commit();
        return msg;

    }catch(const std::exception& e){
        std::cerr << "Error finding message by ID: " << e.what() << std::endl;
        return std::nullopt;
    }
}
std::vector<Message> MySQLMessageRepository::findBySenderId(long long sender_id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    std::vector<Message> messages;
    try{
        sql<<"SELECT id, room_id, sender_id, content, created_at FROM messages WHERE sender_id = :sender_id", soci::use(sender_id), soci::into(messages);
        tr.commit();
    }catch(const std::exception& e){
        std::cerr << "Error finding messages by sender ID: " << e.what() << std::endl;
        messages.clear();
    }
    return messages;
}
std::vector<Message> MySQLMessageRepository::findByRoomId(long long room_id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    std::vector<Message> messages;
    try{
        sql<<"SELECT id, room_id, sender_id, content, created_at FROM messages WHERE room_id = :room_id", soci::use(room_id), soci::into(messages);
        tr.commit();
    }catch(const std::exception& e){
        std::cerr << "Error finding messages by room ID: " << e.what() << std::endl;
        messages.clear();
    }
    return messages;
}
std::vector<Message> MySQLMessageRepository::findByContent(const std::string& content){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
     std::vector<Message> messages;
    try{
        soci::rowset<Message> rs=(sql.prepare<<"SELECT id, room_id, sender_id, content, created_at FROM messages WHERE content LIKE CONCAT('%',:content,'%')", soci::use(content,"content"));
        for(const auto& msg : rs){
            messages.push_back(msg);
        }
        tr.commit();

    }catch(const std::exception& e){
        std::cerr << "Error finding messages by content: " << e.what() << std::endl;
        messages.clear();
    }
    return messages;
}
std::vector<Message> MySQLMessageRepository::findLatestByRoomId(long long roomId, int limit){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    std::vector<Message> messages;
    try{
        soci::rowset<Message> rs=(sql.prepare<<"SELECT id, room_id, sender_id, content, created_at FROM messages WHERE room_id = :room_id ORDER BY created_at DESC LIMIT :limit", soci::use(roomId,"room_id"), soci::use(limit,"limit"));
        for(const auto& msg : rs){
            messages.push_back(msg);
        }
        tr.commit();
    }catch(const std::exception& e){
        std::cerr << "Error finding messages by content: " << e.what() << std::endl;
        messages.clear();
    }
    return messages;
}



