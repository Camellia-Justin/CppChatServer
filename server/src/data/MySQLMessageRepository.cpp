#include "MySQLMessageRepository.h"
#include "ConnectionPool.h"
#include "DataAccess.h"
#include <iostream>
#include <set>


bool MySQLMessageRepository::addMessage(Message& msg){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    try{
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
    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found." << std::endl;
            return false; 
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return false;
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return false;
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw; 
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        return false;
    }
}
bool MySQLMessageRepository::removeMessage(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    try{
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
        sql<<"DELETE FROM messages WHERE id = :id", soci::use(id,"id");
        tr.commit();
        return true;

    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found." << std::endl;
            return false;
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return false;
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return false;
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        return false;
    }
}
std::optional<Message> MySQLMessageRepository::findByMessageId(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    try{
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
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
    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            return std::nullopt;
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return std::nullopt;
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return std::nullopt;
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        return std::nullopt;
    }
}
std::vector<Message> MySQLMessageRepository::findBySenderId(long long sender_id) {
    std::vector<Message> messages;
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
        ConnectionPool::getInstance().getConnection());
    try {
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
    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            messages.clear();
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            messages.clear();
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            messages.clear();
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        messages.clear();
    }
    return messages;
}
std::vector<Message> MySQLMessageRepository::findByRoomId(long long room_id) {
    std::vector<Message> messages;
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
        ConnectionPool::getInstance().getConnection());
    try {
       
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
    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            messages.clear();
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            messages.clear();
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            messages.clear();
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        messages.clear();
    }
    return messages;
}
std::vector<Message> MySQLMessageRepository::findByContent(const std::string& content) {
    std::vector<Message> messages;
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
        ConnectionPool::getInstance().getConnection());
    try {
       
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
    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            messages.clear();
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            messages.clear();
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            messages.clear();
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        messages.clear();
    }
    return messages;
}
std::vector<Message> MySQLMessageRepository::findLatestByRoomId(long long room_id, int limit) {
    std::vector<Message> messages;
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
        ConnectionPool::getInstance().getConnection());
    try {
        
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
    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            messages.clear();
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            messages.clear();
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            messages.clear();
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        messages.clear();
    }

    return messages;
}



