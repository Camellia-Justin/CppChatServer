#pragma once

#include <string>
#include <chrono>
class Room{
public:
    Room() = default;
    virtual ~Room() = default;
    long long getId() const { return id; }
    void setId(long long newId) { id = newId; }

    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }

    long long getCreatorId() const { return creator_id; }
    void setCreatorId(long long newCreatorId) { creator_id = newCreatorId; }

    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at; }
    void setCreatedAt(const std::chrono::system_clock::time_point& newCreatedAt) { created_at = newCreatedAt; }
private:
    long long id;
    std::string name;
    long long creator_id;
    std::chrono::system_clock::time_point created_at;
};
