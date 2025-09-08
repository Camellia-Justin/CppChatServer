#pragma once

#include <vector>
#include "domain/User.h"
#include <optional>
class IUserRepository {
public:
    virtual ~IUserRepository() = default;

    virtual std::optional<User> findByUsername(const std::string& username) = 0;
    virtual std::optional<User> findByUserId(long long id) = 0;
    virtual std::vector<User> getAllUsers() = 0;
    virtual bool updateUser(User& user) = 0;
    virtual bool addUser(User& user) = 0;
    virtual bool removeUser(long long id) = 0;
};