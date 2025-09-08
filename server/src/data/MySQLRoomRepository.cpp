#include "MySQLRoomRepository.h"
#include "ConnectionPool.h"
#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>
#include <iostream>
template<>
struct soci::type_conversion<Room>{
    static void from_base(soci::values& v, indicator,Room& room){
        room.setId(v.get<long long>("id"));
        room.setName(v.get<std::string>("name"));
        room.setCreatorId(v.get<long long>("creator_id"));
        room.setCreatedAt(v.get<std::chrono::system_clock::time_point>("created_at"));
    }
    static void to_base(const Room& room, soci::values& v, soci::indicator& ind){
        v.set("name", room.getName());
        v.set("creator_id", room.getCreatorId());
        ind = soci::i_ok;
    }
};
std::optional<Room> MySQLRoomRepository::findByRoomId(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        Room room;
        sql<<"SELECT id, name, creator_id, created_at FROM rooms WHERE id = :id", soci::use(id), soci::into(room);
        tr.commit();
        return room;
    }catch(const std::exception& e){
        std::cerr << "Error finding room by ID: " << e.what() << std::endl;
        return std::nullopt;
    }
}
std::optional<Room> MySQLRoomRepository::findByRoomName(const std::string& name){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        Room room;
        sql<<"SELECT id, name, creator_id, created_at FROM rooms WHERE name = :name", soci::use(name), soci::into(room);
        tr.commit();
        return room;
    }catch(const std::exception& e){
        std::cerr << "Error finding room by name: " << e.what() << std::endl;
        return std::nullopt;
    }
}
std::optional<Room> MySQLRoomRepository::findByCreatorId(long long creator_id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        Room room;
        sql<<"SELECT id, name, creator_id, created_at FROM rooms WHERE creator_id = :creator_id", soci::use(creator_id), soci::into(room);
        tr.commit();
        return room;
    }catch(const std::exception& e){
        std::cerr << "Error finding room by creator ID: " << e.what() << std::endl;
        return std::nullopt;
    }
}
std::vector<Room> MySQLRoomRepository::getAllRooms(){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    std::vector<Room> rooms;
    try{
        sql<<"SELECT id, name, creator_id, created_at FROM rooms", soci::into(rooms);
        tr.commit();
    }catch(const std::exception& e){
        std::cerr << "Error retrieving all rooms: " << e.what() << std::endl;
        rooms.clear();
    }
    return rooms;
}
bool MySQLRoomRepository::updateRoom(Room& room){
    if(room.getId() <= 0){
        std::cerr << "Invalid room ID for update." << std::endl;
        return false;
    }
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        soci::statement st=(sql.prepare<<"UPDATE rooms SET name = :name WHERE id = :id",soci::use(room));
        st.execute(true);
        if (st.get_affected_rows() > 0) {
            return true;
        } else {
            std::cout << "Update Warning: No room found with ID " << room.getId() 
                      << " to update." << std::endl;
            return false;
        }
    }catch(const std::exception& e){
        std::cerr << "Error updating room: " << e.what() << std::endl;
        return false;
    }
}
bool MySQLRoomRepository::addRoom(Room& room){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        sql<<"INSERT INTO rooms (name, creator_id) VALUES (:name, :creator_id)", soci::use(room);
        long long newId;
        if(!sql.get_last_insert_id("rooms", newId)){
            std::cerr << "Failed to get last insert ID for rooms." << std::endl;
            return false;
        }
        room.setId(newId);
        try{
            std::chrono::system_clock::time_point createdAt;
            sql << "SELECT created_at FROM rooms WHERE id = :id", soci::use(newId), soci::into(createdAt);
            room.setCreatedAt(createdAt);
        }catch(const std::exception& e){ 
            std::cerr << "Error retrieving created_at: " << e.what() << std::endl;
            return false;
        }
        tr.commit();
        return true;
    }catch(const std::exception& e){
        std::cerr << "Error adding room: " << e.what() << std::endl;
        return false;
    }
}
bool MySQLRoomRepository::removeRoom(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        sql<<"DELETE FROM rooms WHERE id = :id", soci::use(id);
        tr.commit();
        return true;
    }catch(const std::exception& e){
        std::cerr << "Error removing room: " << e.what() << std::endl;
        return false;
    }
}