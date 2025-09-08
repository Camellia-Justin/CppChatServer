#include "MySQLMessageRepository.h"
#include "ConnectionPool.h"
#include "DataAccess.h"
#include <iostream>
#include <set>


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
std::vector<Message> MySQLMessageRepository::findBySenderId(long long sender_id) {
    std::vector<Message> messages;
    try {
        auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
                                          ConnectionPool::getInstance().getConnection());
        soci::session& sql = *conWrapper;
        Message row;
        soci::statement st = (sql.prepare <<
            "SELECT id, room_id, sender_id, content, created_at "
            "FROM messages WHERE sender_id = :sender_id",
            soci::into(row),
            soci::use(sender_id, "sender_id"));
        st.execute();
        while (st.fetch()) {
            messages.push_back(row);
        }

    } catch (const std::exception& e) {
        std::cerr << "Database error in findBySenderId: " << e.what() << std::endl;
        messages.clear();
    }
    return messages;
}
std::vector<Message> MySQLMessageRepository::findByRoomId(long long room_id) {
    std::vector<Message> messages;
    try {
        auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
                                          ConnectionPool::getInstance().getConnection());
        soci::session& sql = *conWrapper;
        Message row;
        soci::statement st = (sql.prepare <<
            "SELECT id, room_id, sender_id, content, created_at "
            "FROM messages WHERE room_id = :room_id "
            "ORDER BY created_at ASC",
            soci::into(row),
            soci::use(room_id, "room_id"));
        st.execute();
        while (st.fetch()) {
            messages.push_back(row);
        }
    } catch (const std::exception& e) {
        std::cerr << "Database error in findByRoomId: " << e.what() << std::endl;
        messages.clear();
    }
    return messages;
}
std::vector<Message> MySQLMessageRepository::findByContent(const std::string& content) {
    std::vector<Message> messages;
    try {
        auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
                                          ConnectionPool::getInstance().getConnection());
        soci::session& sql = *conWrapper;
        soci::rowset<Message> rs = (sql.prepare <<
            "SELECT id, room_id, sender_id, content, created_at "
            "FROM messages WHERE content LIKE CONCAT('%', :content, '%')",
            soci::use(content, "content"));
        for (const auto& msg : rs) {
            messages.push_back(msg);
        }
    } catch (const std::exception& e) {
        std::cerr << "Database error in findByContent: " << e.what() << std::endl;
        messages.clear();
    }
    return messages;
}
std::vector<Message> MySQLMessageRepository::findLatestByRoomId(long long roomId, int limit) {
    std::vector<Message> messages;

    try {
        auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
                                          ConnectionPool::getInstance().getConnection());
        soci::session& sql = *conWrapper;
        soci::rowset<Message> rs = (sql.prepare <<
            "SELECT id, room_id, sender_id, content, created_at "
            "FROM messages WHERE room_id = :roomId "
            "ORDER BY created_at DESC LIMIT :limit",
            soci::use(roomId, "roomId"), 
            soci::use(limit, "limitValue"));
        for (const auto& msg : rs) {
            messages.push_back(msg);
        }
    } catch (const std::exception& e) {
        std::cerr << "Database error in findLatestByRoomId: " << e.what() << std::endl;
        messages.clear();
    }

    return messages;
}



