#include "MySQLRoomRepository.h"
#include "ConnectionPool.h"
#include "DataAccess.h"
#include <iostream>

std::optional<Room> MySQLRoomRepository::findByRoomId(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        Room room;
		long long roomid, creator_id;
		std::string roomname;
        std::chrono::system_clock::time_point created_at;
        soci::statement st = (sql.prepare <<
            "SELECT id, name, creator_id, created_at FROM rooms WHERE id = :id",
            soci::use(id), soci::into(roomid),
            soci::into(creator_id),
            soci::into(roomname),
            soci::into(created_at));
        st.execute(true);
        if (st.fetch()) {
			room.setId(roomid);
			room.setCreatorId(creator_id);
			room.setName(roomname);
			room.setCreatedAt(created_at);
            std::cout << "[DEBUG] RoomRepository: Room found. Name: '" << room.getName()
                << "', ID: " << room.getId() << std::endl;
            return room;
        }
        else {
            std::cout << "[DEBUG] RoomRepository: Room with id '" << id << "' not found in database." << std::endl;
            return std::nullopt;
        }
    }catch(const std::exception& e){
        std::cerr << "Error finding room by ID: " << e.what() << std::endl;
        return std::nullopt;
    }
}
std::optional<Room> MySQLRoomRepository::findByRoomName(const std::string& name){
    std::cout << "[DEBUG] RoomRepository: Searching for name: '" << name << "'" << std::endl;
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        long long roomid, creator_id;
        std::string roomname;
        std::tm created_at_tm = {};
        soci::indicator id_ind, roomname_ind, creator_id_ind, created_at_ind;
        soci::statement st = (sql.prepare <<
            "SELECT id, name, creator_id, created_at FROM rooms WHERE name = :name",
            soci::use(name), soci::into(roomid,id_ind), soci::into(roomname,roomname_ind), soci::into(creator_id,creator_id_ind), soci::into(created_at_tm,created_at_ind));
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
        soci::statement st = (sql.prepare <<
            "SELECT id, name, creator_id, created_at FROM rooms WHERE creator_id = :creator_id",
            soci::use(creator_id), soci::into(room));
        st.execute(true);
        if (st.fetch()) {
            std::cout << "[DEBUG] RoomRepository: Room found. Name: '" << room.getName()
                << "', ID: " << room.getId() << std::endl;
            return room;
        }
        else {
            std::cout << "[DEBUG] RoomRepository: Room with creatorId '" << creator_id << "' not found in database." << std::endl;
            return std::nullopt;
        }
    }catch(const std::exception& e){
        std::cerr << "Error finding room by creator ID: " << e.what() << std::endl;
        return std::nullopt;
    }
}
// 保持你的命名和函数签名
std::vector<Room> MySQLRoomRepository::getAllRooms() {
    std::vector<Room> rooms;

    try {
        auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
                                          ConnectionPool::getInstance().getConnection());
        soci::session& sql = *conWrapper;

        // SELECT 查询，不需要事务

        // 1. 创建一个用于接收“单行”结果的临时 Room 对象
        Room row;

        // 2. 准备一个 statement，并将 into 绑定到这个“单行”对象上
        soci::statement st = (sql.prepare <<
            "SELECT id, name, creator_id, created_at FROM rooms",
            soci::into(row));

        // 3. 执行查询
        st.execute();

        // 4. 使用 while 循环和 statement::fetch() 来逐行获取数据
        while (st.fetch()) {
            rooms.push_back(row);
        }

    } catch (const std::exception& e) {
        std::cerr << "Database error in getAllRooms: " << e.what() << std::endl;
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
        soci::statement st=(sql.prepare<<"UPDATE rooms SET name = :name WHERE id = :id",
            soci::use(room.getName(),"name"));
        st.execute(true);
        if (st.get_affected_rows() > 0) {
            tr.commit();
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
                // 手动转换为 time_point
                room.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&created_tm)));
            }
            else {
                std::cerr << "[WARNING] created_at is NULL or no data for id=" << newId << std::endl;
                room.setCreatedAt(std::chrono::system_clock::now());
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error retrieving created_at: " << e.what() << std::endl;
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