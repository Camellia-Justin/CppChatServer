#include "MySQLUserRepository.h"
#include "ConnectionPool.h"
#include "DataAccess.h"
#include <iostream>
#include <vector>
#include <optional>


std::optional<User> MySQLUserRepository::findByUsername(const std::string& username) {
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    try {
        
        soci::session& sql = *conWrapper;
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
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            return std::nullopt;
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return std::nullopt;
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return std::nullopt;
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        return std::nullopt;
    }
}
std::optional<User> MySQLUserRepository::findByUserId(long long id){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    try{
        
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
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
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            return std::nullopt;
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return std::nullopt;
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return std::nullopt;
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        return std::nullopt;
    }
}
std::vector<User> MySQLUserRepository::getAllUsers() {
    std::vector<User> users;
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    try {
        soci::session& sql = *conWrapper;

        long long id_val;
        std::string username_val;
        std::string hashed_password_val;
        std::string salt_val;
        std::tm created_at_tm = {};

        soci::indicator id_ind, username_ind, hashed_password_ind, salt_ind, created_at_ind;

        // 准备语句：逐字段绑定
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
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            users.clear();
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            users.clear();
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            users.clear();
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        users.clear();
    }

    return users;
}
bool MySQLUserRepository::updateUser(User& user){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    try{
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
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
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            return false;
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return false;
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return false;
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        return false;
    }
}
bool MySQLUserRepository::addUser(User& user){
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    try{
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
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
    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            return false;
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return false;
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return false;
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch(const std::exception& e){
        conWrapper.markAsInvalid();
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
    try{
        soci::session& sql = *conWrapper;
        soci::transaction tr(sql);
        sql << "DELETE FROM users WHERE id = :id", soci::use(id,"id");
        tr.commit();
        return true;

    }
    catch (const soci::soci_error& e) {
        soci::soci_error::error_category category = e.get_error_category();
        if (category == soci::soci_error::error_category::no_data) {
            std::cout << "[DEBUG] No data found for query." << std::endl;
            return false;
        }
        else if (category == soci::soci_error::error_category::connection_error) {
            std::cerr << "[ERROR] Connection error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return false;
        }
        else if (category == soci::soci_error::error_category::system_error) {
            std::cerr << "[ERROR] System/Driver error: " << e.what() << std::endl;
            conWrapper.markAsInvalid();
            return false;
        }
        else {
            std::cerr << "[ERROR] Database operation error: " << e.what()
                << " (Category: " << category << ")" << std::endl;
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected standard exception: " << e.what() << std::endl;
        conWrapper.markAsInvalid();
        return false;
    }
}
