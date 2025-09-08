#include "MySQLUserRepository.h"
#include "ConnectionPool.h"
#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>
#include <iostream>
#include <vector>
#include <optional>
template<>
struct soci::type_conversion<User>{
    static void from_base(soci::values& v, indicator,User& user){
        user.setId(v.get<long long>("id"));
        user.setUsername(v.get<std::string>("username"));
        user.setHashedPassword(v.get<std::string>("hashed_password"));
        user.setSalt(v.get<std::string>("salt"));
        user.setCreatedAt(v.get<std::chrono::system_clock::time_point>("created_at"));
    }
    static void to_base(const User& user, soci::values& v, soci::indicator& ind){
        v.set("username", user.getUsername());
        v.set("hashed_password", user.getHashedPassword());
        v.set("salt", user.getSalt());
        ind = soci::i_ok;
    }
};
std::optional<User> MySQLUserRepository::findByUsername(const std::string& username){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        User user;
        sql<<"SELECT id, username, hashed_password, salt, created_at FROM users WHERE username = :username", soci::use(username), soci::into(user);
        tr.commit();
        return user;
    }catch(const std::exception& e){
        std::cerr << "Error finding user by username: " << e.what() << std::endl;
        return std::nullopt;
    }
}
std::optional<User> MySQLUserRepository::findByUserId(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        User user;
        sql<<"SELECT id, username, hashed_password, salt, created_at FROM users WHERE id = :id", soci::use(id), soci::into(user);
        tr.commit();
        return user;
    }catch(const std::exception& e){
        std::cerr << "Error finding user by ID: " << e.what() << std::endl;
        return std::nullopt;
    }
}
std::vector<User> MySQLUserRepository::getAllUsers(){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    std::vector<User> users;
    try{
        sql<<"SELECT id, username, hashed_password, salt, created_at FROM users", soci::into(users);
        tr.commit();
    }catch(const std::exception& e){
        std::cerr << "Error retrieving all users: " << e.what() << std::endl;
        users.clear();
    }
    return users;
}
bool MySQLUserRepository::updateUser(User& user){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
    soci::statement st=(sql.prepare<<"UPDATE users SET username = :username, hashed_password = :hashed_password, salt = :salt WHERE id = :id",soci::use(user));
    st.execute(true);
        if (st.get_affected_rows() > 0) {
            return true;
        } else {
            std::cout << "Update Warning: No user found with ID " << user.getId() 
                  << " to update." << std::endl;
            return false;
        }
    }catch(const std::exception& e){
        std::cerr << "Error preparing update statement: " << e.what() << std::endl;
        return false;
    }
}
bool MySQLUserRepository::addUser(User& user){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        sql<<"INSERT INTO users (username, hashed_password, salt) VALUES (:username, :hashed_password, :salt)", soci::use(user);
        long long newId;
        if(!sql.get_last_insert_id("users", newId)){
            std::cerr << "Error retrieving last insert ID after adding user." << std::endl;
            return false;
        }
        user.setId(newId);
        try{
            std::chrono::system_clock::time_point createdAt;
            sql << "SELECT created_at FROM users WHERE id = :id", soci::use(newId), soci::into(createdAt);
            user.setCreatedAt(createdAt);
            tr.commit();
            return true;
        }catch(const std::exception& e){
            std::cerr << "Error retrieving created_at: " << e.what() << std::endl;
            return false;
        }
    }catch(const std::exception& e){
        std::cerr << "Error preparing insert statement: " << e.what() << std::endl;
        return false;
    }
}
bool MySQLUserRepository::removeUser(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        sql<<"DELETE FROM users WHERE id = :id", soci::use(id);
        tr.commit();
        return true;

    }catch(const std::exception& e){
        std::cerr << "Error removing user: " << e.what() << std::endl;
        return false;
    }
}
