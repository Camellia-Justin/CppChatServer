#pragma once

#include <vector>
#include <optional>
#include "domain/Room.h"

class IRoomRepository {
public:
    virtual ~IRoomRepository() = default;

    virtual std::optional<Room> findByRoomId(long long id) = 0;
    virtual std::optional<Room> findByRoomName(const std::string& name) = 0;
    virtual std::optional<Room> findByCreatorId(long long creator_id) = 0;
    virtual std::vector<Room> getAllRooms() = 0;
    virtual bool updateRoom(Room& room) = 0;
    virtual bool addRoom(Room& room) = 0;
    virtual bool removeRoom(long long id) = 0;
};
