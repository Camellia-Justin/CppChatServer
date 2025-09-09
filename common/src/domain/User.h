#pragma once

#include <string>
#include <memory>
#include <chrono>

class User{
public:
    User() = default;
    virtual ~User() = default;
    long long getId() const { return id; }
    void setId(long long newId) { id = newId; }

    const std::string& getUsername() const { return username; }
    void setUsername(const std::string& newUsername) { username = newUsername; }
    
    const std::string& getHashedPassword() const { return hashed_password; }
    void setHashedPassword(const std::string& newHashedPassword) { hashed_password = newHashedPassword; }
    
    const std::string& getSalt() const { return salt; }
    void setSalt(const std::string& newSalt) { salt = newSalt; }
    
    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at; }
    void setCreatedAt(const std::chrono::system_clock::time_point& newCreatedAt) { created_at = newCreatedAt; }
private:
    long long id;
    std::string username;
    std::string hashed_password;
    std::string salt;
    std::chrono::system_clock::time_point created_at;

};