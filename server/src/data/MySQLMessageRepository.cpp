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
            soci::use(msg.getRoomId(), "room_id"),
            soci::use(msg.getSenderId(), "sender_id"),
            soci::use(msg.getContent(), "content");
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
        sql<<"DELETE FROM messages WHERE id = :id", soci::use(id,"id");
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
        long long id_val,room_id_val,sender_id_val;
        std::string content_val;
        std::tm created_at_tm = {};
		soci::indicator id_ind, room_id_ind, sender_id_ind, content_ind, created_at_ind;
        soci::statement st = (sql.prepare << "SELECT id, room_id, sender_id, content, created_at FROM messages WHERE id = :id", 
            soci::use(id), soci::into(id_val, id_ind), 
            soci::into(room_id_val, room_id_ind), 
            soci::into(sender_id_val, sender_id_ind), 
            soci::into(content_val, content_ind), 
            soci::into(created_at_tm, created_at_ind));
        st.execute(true);
        if (sql.got_data() && id_ind == soci::i_ok) {
            Message msg;
            msg.setId(id_val);
            msg.setRoomId(room_id_val);
            msg.setSenderId(sender_id_val);
            msg.setContent(content_val);
            if (created_at_ind == soci::i_ok) {
                msg.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&created_at_tm)));
            }
            else {
                msg.setCreatedAt(std::chrono::system_clock::now());
            }
            std::cout << "[DEBUG] MessageRepository: Message found. ID: " << msg.getId() << std::endl;
            return msg;
        }
        else {
            std::cout << "[DEBUG] MessageRepository: message not found in database." << std::endl;
            return std::nullopt;
        }
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

        long long id_val, room_id_val, sender_id_val;
        std::string content_val;
        std::tm created_at_tm = {};
        soci::indicator id_ind, room_id_ind, sender_id_ind, content_ind, created_at_ind;

        soci::statement st = (sql.prepare <<
            "SELECT id, room_id, sender_id, content, created_at "
            "FROM messages WHERE sender_id = :sender_id",
            soci::use(sender_id, "sender_id"),
            soci::into(id_val, id_ind),
            soci::into(room_id_val, room_id_ind),
            soci::into(sender_id_val, sender_id_ind),
            soci::into(content_val, content_ind),
            soci::into(created_at_tm, created_at_ind));
        st.execute();

        while (st.fetch()) {
            Message msg;
            msg.setId(id_val);
            msg.setRoomId(room_id_val);
            msg.setSenderId(sender_id_val);
            msg.setContent(content_val);
            if (created_at_ind == soci::i_ok) {
                std::time_t tt = std::mktime(&created_at_tm);
                if (tt != -1) {
                    msg.setCreatedAt(std::chrono::system_clock::from_time_t(tt));
                }
                else {
                    msg.setCreatedAt(std::chrono::system_clock::now());
                }
            }
            else {
                msg.setCreatedAt(std::chrono::system_clock::now());
            }
            messages.push_back(msg);
            
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

        long long id_val, room_id_val, sender_id_val;
        std::string content_val;
        std::tm created_at_tm = {};
        soci::indicator id_ind, room_id_ind, sender_id_ind, content_ind, created_at_ind;

        soci::statement st = (sql.prepare <<
            "SELECT id, room_id, sender_id, content, created_at "
            "FROM messages WHERE room_id = :room_id "
            "ORDER BY created_at ASC",
            soci::use(room_id, "room_id"),
            soci::into(id_val, id_ind),
            soci::into(room_id_val, room_id_ind),
            soci::into(sender_id_val, sender_id_ind),
            soci::into(content_val, content_ind),
            soci::into(created_at_tm, created_at_ind));
        st.execute();
        while (st.fetch()) {
            Message msg;
            msg.setId(id_val);
            msg.setRoomId(room_id_val);
            msg.setSenderId(sender_id_val);
            msg.setContent(content_val);

            if (created_at_ind == soci::i_ok) {
                std::time_t tt = std::mktime(&created_at_tm);
                if (tt != -1) {
                    msg.setCreatedAt(std::chrono::system_clock::from_time_t(tt));
                }
                else {
                    msg.setCreatedAt(std::chrono::system_clock::now());
                }
            }
            else {
                msg.setCreatedAt(std::chrono::system_clock::now());
            }
            messages.push_back(msg);
            
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

        long long id_val, room_id_val, sender_id_val;
        std::string content_val;
        std::tm created_at_tm = {};
        soci::indicator id_ind, room_id_ind, sender_id_ind, content_ind, created_at_ind;

        soci::statement st = (sql.prepare <<
            "SELECT id, room_id, sender_id, content, created_at "
            "FROM messages WHERE content LIKE CONCAT('%', :content, '%')",
            soci::use(content, "content"),
            soci::into(id_val, id_ind),
            soci::into(room_id_val, room_id_ind),
            soci::into(sender_id_val, sender_id_ind),
            soci::into(content_val, content_ind),
            soci::into(created_at_tm, created_at_ind));
        st.execute();
        while (st.fetch()) {
            Message msg;
            msg.setId(id_val);
            msg.setRoomId(room_id_val);
            msg.setSenderId(sender_id_val);
            msg.setContent(content_val);
            if (created_at_ind == soci::i_ok) {
                std::time_t tt = std::mktime(&created_at_tm);
                if (tt != -1) {
                    msg.setCreatedAt(std::chrono::system_clock::from_time_t(tt));
                }
                else {
                    msg.setCreatedAt(std::chrono::system_clock::now());
                }
            }
            else {
                msg.setCreatedAt(std::chrono::system_clock::now());
            }

            messages.push_back(msg);
        }
    } catch (const std::exception& e) {
        std::cerr << "Database error in findByContent: " << e.what() << std::endl;
        messages.clear();
    }
    return messages;
}
std::vector<Message> MySQLMessageRepository::findLatestByRoomId(long long room_id, int limit) {
    std::vector<Message> messages;

    try {
        auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
            ConnectionPool::getInstance().getConnection());
        soci::session& sql = *conWrapper;

        long long id_val, room_id_val, sender_id_val;
        std::string content_val;
        std::tm created_at_tm = {};
        soci::indicator id_ind, room_id_ind, sender_id_ind, content_ind, created_at_ind;

        soci::statement st = (sql.prepare <<
            "SELECT id, room_id, sender_id, content, created_at "
            "FROM messages WHERE room_id = :room_id "
            "ORDER BY created_at DESC LIMIT :limitValue",
            soci::use(room_id, "room_id"),
            soci::use(limit, "limitValue"),
            soci::into(id_val, id_ind),
            soci::into(room_id_val, room_id_ind),
            soci::into(sender_id_val, sender_id_ind),
            soci::into(content_val, content_ind),
            soci::into(created_at_tm, created_at_ind));

        st.execute();

        while (st.fetch()) {
            Message msg;
            msg.setId(id_val);
            msg.setRoomId(room_id_val);
            msg.setSenderId(sender_id_val);
            msg.setContent(content_val);

            if (created_at_ind == soci::i_ok) {
                std::time_t tt = std::mktime(&created_at_tm);
                if (tt != -1) {
                    msg.setCreatedAt(std::chrono::system_clock::from_time_t(tt));
                }
                else {
                    msg.setCreatedAt(std::chrono::system_clock::now());
                }
            }
            else {
                msg.setCreatedAt(std::chrono::system_clock::now());
            }

            messages.push_back(msg);
        }
    } catch (const std::exception& e) {
        std::cerr << "Database error in findLatestByRoomId: " << e.what() << std::endl;
        messages.clear();
    }

    return messages;
}



