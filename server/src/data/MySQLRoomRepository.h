#pragma once

#include "IRoomRepository.h"

class MySQLRoomRepository : public IRoomRepository {
public:
    MySQLRoomRepository() = default;
    ~MySQLRoomRepository() = default;

    std::optional<Room> findByRoomId(long long id);
    std::optional<Room> findByRoomName(const std::string& name);
    std::optional<Room> findByCreatorId(long long creator_id);
    std::vector<Room> getAllRooms();
    bool updateRoom(Room& room);
    bool addRoom(Room& room);
    bool removeRoom(long long id);
};
