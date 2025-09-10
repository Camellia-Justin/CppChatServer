#include "MySQLUserRepository.h"
#include "ConnectionPool.h"
#include "DataAccess.h"
#include <iostream>
#include <vector>
#include <optional>


std::optional<User> MySQLUserRepository::findByUsername(const std::string& username) {
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;

    try {
        int db_id;
        std::string db_username, hashed_password, salt;
        std::tm created_at_tm = {};
        soci::indicator id_ind, username_ind, password_ind, salt_ind, created_ind;

        sql << "SELECT id, username, hashed_password, salt, created_at FROM users WHERE username = :username",
            soci::use(username),
            soci::into(db_id, id_ind),
            soci::into(db_username, username_ind),
            soci::into(hashed_password, password_ind),
            soci::into(salt, salt_ind),
            soci::into(created_at_tm, created_ind);

        if (sql.got_data() && id_ind == soci::i_ok) {
            User user;
            user.setId(static_cast<long long>(db_id));
            user.setUsername(db_username);
            user.setHashedPassword(hashed_password);
            user.setSalt(salt);

            if (created_ind == soci::i_ok) {
                user.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&created_at_tm)));
            }
            else {
                user.setCreatedAt(std::chrono::system_clock::now());
            }

            std::cout << "[DEBUG] UserRepository: User found with ID: " << user.getId() << std::endl;
            return user;
        }

        std::cout << "[DEBUG] UserRepository: User not found." << std::endl;
        return std::nullopt;
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] UserRepository: " << e.what() << std::endl;
        return std::nullopt;
    }
}
std::optional<User> MySQLUserRepository::findByUserId(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        int db_id;
        std::string db_username, hashed_password, salt;
        std::tm created_at_tm = {};
        soci::indicator id_ind, username_ind, password_ind, salt_ind, created_ind;

        sql << "SELECT id, username, hashed_password, salt, created_at FROM users WHERE id = :id",
            soci::use(id),
            soci::into(db_id, id_ind),
            soci::into(db_username, username_ind),
            soci::into(hashed_password, password_ind),
            soci::into(salt, salt_ind),
            soci::into(created_at_tm, created_ind);

        if (sql.got_data() && id_ind == soci::i_ok) {
            User user;
            user.setId(static_cast<long long>(db_id));
            user.setUsername(db_username);
            user.setHashedPassword(hashed_password);
            user.setSalt(salt);

            if (created_ind == soci::i_ok) {
                user.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&created_at_tm)));
            }
            else {
                user.setCreatedAt(std::chrono::system_clock::now());
            }
            std::cout << "[DEBUG] UserRepository: User found with ID: " << user.getId() << std::endl;
            return user;
        }
        std::cout << "[DEBUG] UserRepository: User not found." << std::endl;
        return std::nullopt;
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] UserRepository: " << e.what() << std::endl;
        return std::nullopt;
    }
}
std::vector<User> MySQLUserRepository::getAllUsers() {
    std::vector<User> users;

    try {
        auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
            ConnectionPool::getInstance().getConnection());
        soci::session& sql = *conWrapper;

        long long id_val;
        std::string username_val;
        std::string hashed_password_val;
        std::string salt_val;
        std::tm created_at_tm = {};

        soci::indicator id_ind, username_ind, hashed_password_ind, salt_ind, created_at_ind;

        soci::statement st = (sql.prepare <<
            "SELECT id, username, hashed_password, salt, created_at FROM users",
            soci::into(id_val, id_ind),
            soci::into(username_val, username_ind),
            soci::into(hashed_password_val, hashed_password_ind),
            soci::into(salt_val, salt_ind),
            soci::into(created_at_tm, created_at_ind));
        st.execute();
        while (st.fetch()) { 

            User user;
            user.setId(id_val);
            user.setUsername(username_val);
            user.setHashedPassword(hashed_password_val);
            user.setSalt(salt_val);
            if (created_at_ind == soci::i_ok) {
                std::time_t tt = std::mktime(&created_at_tm);
                if (tt != -1) {
                    user.setCreatedAt(std::chrono::system_clock::from_time_t(tt));
                }
                else {
                    user.setCreatedAt(std::chrono::system_clock::now());
                }
            }
            else {
                user.setCreatedAt(std::chrono::system_clock::now()); 
            }

            users.push_back(user);
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Database error in getAllUsers: " << e.what() << std::endl;
        users.clear();
    }

    return users;
}
bool MySQLUserRepository::updateUser(User& user){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        soci::statement st=(sql.prepare<<"UPDATE users SET username = :username, hashed_password = :hashed_password, salt = :salt WHERE id = :id",
        soci::use(user.getUsername(),"username"),
        soci::use(user.getHashedPassword(),"hashed_password"),
        soci::use(user.getSalt(),"salt"),
        soci::use(user.getId(),"id"));
        st.execute(true);
        if (st.get_affected_rows() > 0) {
            tr.commit();
            return true;
        }
        else {
            std::cout << "Update Warning: No user found with ID " << user.getId() << std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error updating user: " << e.what() << std::endl;
        return false;
    }
}
bool MySQLUserRepository::addUser(User& user){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        sql<<"INSERT INTO users (username, hashed_password, salt) VALUES (:username, :hashed_password, :salt)",
    		soci::use(user.getUsername(), "username"),
            soci::use(user.getHashedPassword(), "hashed_password"),
            soci::use(user.getSalt(), "salt");
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
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "!!! DATABASE EXCEPTION CAUGHT in addUser !!!" << std::endl;
        std::cerr << "!!! Exception Type: " << typeid(e).name() << std::endl;
        std::cerr << "!!! Exception what(): " << e.what() << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        return false;
    }
}
bool MySQLUserRepository::removeUser(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        sql << "DELETE FROM users WHERE id = :id", soci::use(id,"id");
        tr.commit();
        return true;

    }catch(const std::exception& e){
        std::cerr << "Error removing user: " << e.what() << std::endl;
        return false;
    }
}
