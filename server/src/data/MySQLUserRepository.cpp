#include "MySQLUserRepository.h"
#include "ConnectionPool.h"
#include "DataAccess.h"
#include <iostream>
#include <vector>
#include <optional>


std::optional<User> MySQLUserRepository::findByUsername(const std::string& username) {
    std::cout << "[DEBUG] UserRepository: Searching for username: '" << username << "'" << std::endl;
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;

    try {
        int id;
        std::string db_username, hashed_password, salt;
        std::tm created_at_tm = {};
        soci::indicator id_ind, username_ind, password_ind, salt_ind, created_ind;

        sql << "SELECT id, username, hashed_password, salt, created_at FROM users WHERE username = :username",
            soci::use(username),
            soci::into(id, id_ind),
            soci::into(db_username, username_ind),
            soci::into(hashed_password, password_ind),
            soci::into(salt, salt_ind),
            soci::into(created_at_tm, created_ind);

        if (sql.got_data() && id_ind == soci::i_ok) {
            User user;
            user.setId(static_cast<long long>(id));
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
        User user;
        soci::statement st = (sql.prepare <<
            "SELECT id, username, hashed_password, salt, created_at FROM users WHERE id = :id",
            soci::use(id), soci::into(user));
        st.execute(true);
        if (st.fetch()) {
            std::cout << "[DEBUG] UserRepository: User found. Name: '" << user.getUsername()
                << "', ID: " << user.getId() << std::endl;
            return user;
        }
        else {
            std::cout << "[DEBUG] UserRepository: User with id '" << id << "' not found in database." << std::endl;
            return std::nullopt;
        }
    }catch(const std::exception& e){
        std::cerr << "Error finding user by ID: " << e.what() << std::endl;
        return std::nullopt;
    }
}
// 保持你的命名和函数签名
std::vector<User> MySQLUserRepository::getAllUsers() {
    std::vector<User> users;

    try {
        auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(),
                                          ConnectionPool::getInstance().getConnection());
        soci::session& sql = *conWrapper;
        
        // SELECT 查询，不需要事务

        // 1. 创建一个用于接收“单行”结果的临时 User 对象
        User row;

        // 2. 准备一个 statement，并将 into 绑定到这个“单行”对象上
        soci::statement st = (sql.prepare <<
            "SELECT id, username, hashed_password, salt, created_at FROM users",
            soci::into(row));

        // 3. 执行查询
        st.execute();

        // 4. 使用 while 循环和 statement::fetch() 来逐行获取数据
        while (st.fetch()) {
            users.push_back(row);
        }

    } catch (const std::exception& e) {
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
        soci::use(user.getSalt(),"salt"));
    st.execute(true);
        if (st.get_affected_rows() > 0) {
            tr.commit();
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
    std::cout << "[DEBUG] UserRepository: Entered create. user address: "
        << &user << ", username: '" << user.getUsername() << "'" << std::endl;
    auto conWrapper = ConnectionWrapper(&ConnectionPool::getInstance(), ConnectionPool::getInstance().getConnection());
    soci::session& sql = *conWrapper;
    soci::transaction tr(sql);
    try{
        std::cout << "[DEBUG] Before soci::use, username is: '" << user.getUsername() << "'" << std::endl;
        std::cout << "[DEBUG] Before soci::use, hash is: '" << user.getHashedPassword() << "'" << std::endl;
        std::cout << "[DEBUG] Before soci::use, salt is: '" << user.getSalt() << "'" << std::endl;

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
        sql<<"DELETE FROM users WHERE id = :id", soci::use(id);
        tr.commit();
        return true;

    }catch(const std::exception& e){
        std::cerr << "Error removing user: " << e.what() << std::endl;
        return false;
    }
}
