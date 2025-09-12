#include "MySQLRoomRepository.h"
#include "ConnectionPool.h"
#include "DataAccess.h"
#include <iostream>

std::optional<Room> MySQLRoomRepository::findByRoomId(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    try{
        
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
        long long roomid, creator_id;
        std::string roomname;
        std::tm created_at_tm = {};
        soci::indicator id_ind, roomname_ind, creator_id_ind, created_at_ind;
        soci::statement st = (sql.prepare <<
            "SELECT id, name, creator_id, created_at FROM rooms WHERE name = :name",
            soci::use(id), soci::into(roomid, id_ind),
            soci::into(roomname, roomname_ind),
            soci::into(creator_id, creator_id_ind),
            soci::into(created_at_tm, created_at_ind));
        st.execute(true);
        if (sql.got_data() && id_ind == soci::i_ok) {
            Room room;
            room.setId(roomid);
            room.setCreatorId(creator_id);
            room.setName(roomname);
            if (created_at_ind == soci::i_ok) {
                room.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&created_at_tm)));
            }
            else {
                room.setCreatedAt(std::chrono::system_clock::now());
            }
            std::cout << "[DEBUG] RoomRepository: Room found. Name: '" << room.getName()
                << "', ID: " << room.getId() << std::endl;
            return room;
        }
        std::cout << "[DEBUG] RoomRepository: Room not found." << std::endl;
        return std::nullopt;
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
std::optional<Room> MySQLRoomRepository::findByRoomName(const std::string& name){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    try{
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
        long long roomid, creator_id;
        std::string roomname;
        std::tm created_at_tm = {};
        soci::indicator id_ind, roomname_ind, creator_id_ind, created_at_ind;
        soci::statement st = (sql.prepare <<
            "SELECT id, name, creator_id, created_at FROM rooms WHERE name = :name",
            soci::use(name), soci::into(roomid, id_ind),
            soci::into(roomname, roomname_ind),
            soci::into(creator_id, creator_id_ind),
            soci::into(created_at_tm, created_at_ind));
        st.execute(true);
        if (sql.got_data() && id_ind == soci::i_ok) {
            Room room;
			room.setId(roomid);
			room.setCreatorId(creator_id);
			room.setName(roomname);
            if (created_at_ind == soci::i_ok) {
                room.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&created_at_tm)));
            }
            else {
                room.setCreatedAt(std::chrono::system_clock::now());
            }
            std::cout << "[DEBUG] RoomRepository: Room found. Name: '" << room.getName()
                << "', ID: " << room.getId() << std::endl;
            return room;
        }
        std::cout << "[DEBUG] RoomRepository: Room not found." << std::endl;
        return std::nullopt;

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
std::optional<Room> MySQLRoomRepository::findByCreatorId(long long creatorid){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    try{
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
        long long roomid, creator_id;
        std::string roomname;
        std::tm created_at_tm = {};
        soci::indicator id_ind, roomname_ind, creator_id_ind, created_at_ind;
        soci::statement st = (sql.prepare <<
            "SELECT id, name, creator_id, created_at FROM rooms WHERE creator_id = :creator_id",
            soci::use(creatorid), soci::into(roomid, id_ind),
            soci::into(roomname, roomname_ind),
            soci::into(creator_id, creator_id_ind),
            soci::into(created_at_tm, created_at_ind));
        st.execute(true);
        if (sql.got_data() && id_ind == soci::i_ok) {
            Room room;
            room.setId(roomid);
            room.setCreatorId(creator_id);
            room.setName(roomname);
            if (created_at_ind == soci::i_ok) {
                room.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&created_at_tm)));
            }
            else {
                room.setCreatedAt(std::chrono::system_clock::now());
            }
            std::cout << "[DEBUG] RoomRepository: Room found. Name: '" << room.getName()
                << "', ID: " << room.getId() << std::endl;
            return room;
        }
        std::cout << "[DEBUG] RoomRepository: Room not found." << std::endl;
        return std::nullopt;
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
std::vector<Room> MySQLRoomRepository::getAllRooms() {
    std::vector<Room> rooms;
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
        ConnectionPool::getInstance().getConnection());
    try {
        
        soci::session& sql = *conWrapper;

        long long id_val;
        std::string name_val;
        long long creator_id_val;
        std::tm created_at_tm = {};
        soci::indicator id_ind, name_ind, creator_id_ind, created_at_ind;
        // 准备语句
        soci::statement st = (sql.prepare <<
            "SELECT id, name, creator_id, created_at FROM rooms",
            soci::into(id_val, id_ind),
            soci::into(name_val, name_ind),
            soci::into(creator_id_val, creator_id_ind),
            soci::into(created_at_tm, created_at_ind));

        st.execute(); // 执行查询

        while (st.fetch()) {
            Room room;
            room.setId(id_val);
            room.setName(name_val);
            room.setCreatorId(creator_id_val);

            if (created_at_ind == soci::i_ok) {
                std::time_t tt = std::mktime(&created_at_tm);
                if (tt != -1) {
                    room.setCreatedAt(std::chrono::system_clock::from_time_t(tt));
                }
                else {
                    room.setCreatedAt(std::chrono::system_clock::now());
                }
            }
            else {
                room.setCreatedAt(std::chrono::system_clock::now());
            }

            rooms.push_back(room);
        }
    } 
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            rooms.clear();
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            rooms.clear();
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            rooms.clear();
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
        rooms.clear();
    }

    return rooms;
}
bool MySQLRoomRepository::updateRoom(Room& room){
    if(room.getId() <= 0){
        std::cerr << "Invalid room ID for update." << std::endl;
        return false;
    }
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
        ConnectionPool::getInstance().getConnection());
    try{
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
        soci::statement st=(sql.prepare<<"UPDATE rooms SET name = :name WHERE id = :id",
            soci::use(room.getName(),"name"),
		    soci::use(room.getId(), "id"));
        st.execute(true);
        if (st.get_affected_rows() > 0) {
            tr.commit();
            return true;
        }
        else {
            tr.commit();
            std::cout << "Update Warning: No room found with ID " << room.getId() << std::endl;
            return false;
        }
    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
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
bool MySQLRoomRepository::addRoom(Room& room){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
        ConnectionPool::getInstance().getConnection());
    try{
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
        sql << "INSERT INTO rooms (name, creator_id) VALUES (:name, :creator_id)",
            soci::use(room.getName(), "name"),
            soci::use(room.getCreatorId(), "creator_id");
        long long newId;
        if(!sql.get_last_insert_id("rooms", newId)){
            std::cerr << "Failed to get last insert ID for rooms." << std::endl;
            return false;
        }
        room.setId(newId);
        try {
            std::tm created_tm = {};
            soci::indicator ind;

            sql << "SELECT created_at FROM rooms WHERE id = :id",
                soci::use(newId),
                soci::into(created_tm, ind);
            if (ind == soci::i_ok) {
                room.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&created_tm)));
            }
            else {
                std::cerr << "[WARNING] created_at is NULL or no data for id=" << newId << std::endl;
                room.setCreatedAt(std::chrono::system_clock::now());
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error retrieving created_at: " << e.what() << std::endl;
            return false;
        }
        tr.commit();
        return true;
    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
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
bool MySQLRoomRepository::removeRoom(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
        ConnectionPool::getInstance().getConnection());
    try{
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
        sql<<"DELETE FROM rooms WHERE id = :id", soci::use(id,"id");
        tr.commit();
        return true;
    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
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