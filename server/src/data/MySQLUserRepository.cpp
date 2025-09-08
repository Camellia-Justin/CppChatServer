#include "MySQLUserRepository.h"
#include "ConnectionPool.h"
#include "DataAccess.h"
#include <iostream>
#include <vector>
#include <optional>


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
