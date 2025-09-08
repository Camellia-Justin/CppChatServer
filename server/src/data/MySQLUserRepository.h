#pragma once

#include "IUserRepository.h"

class MySQLUserRepository : public IUserRepository {
public:
    MySQLUserRepository() = default;
    ~MySQLUserRepository() = default;

    std::optional<User> findByUsername(const std::string& username);
    std::optional<User> findByUserId(long long id);
    std::vector<User> getAllUsers();
    bool updateUser(User& user);
    bool addUser(User& user);
    bool removeUser(long long id);
};
