#pragma once

#include <string>
#include <memory>
#include <chrono>

class User{
public:
    User() = default;
    ~User() = default;
    int getId() const { return id; }
    void setId(int newId) { id = newId; }

    std::string getUsername() const { return username; }
    void setUsername(const std::string& newUsername) { username = newUsername; }
    
    std::string getHashedPassword() const { return hashed_password; }
    void setHashedPassword(const std::string& newHashedPassword) { hashed_password = newHashedPassword; }
    
    std::string getSalt() const { return salt; }
    void setSalt(const std::string& newSalt) { salt = newSalt; }
    
    std::chrono::system_clock::time_point getCreatedAt() const { return created_at; }
    void setCreatedAt(const std::chrono::system_clock::time_point& newCreatedAt) { created_at = newCreatedAt; }
private:
    long long id;
    std::string username;
    std::string hashed_password;
    std::string salt;
    std::chrono::system_clock::time_point created_at;

};