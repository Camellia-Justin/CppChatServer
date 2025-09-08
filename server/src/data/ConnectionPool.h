#pragma once

#include <soci/soci.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
class ConnectionPool {
public:
    ConnectionPool()=default;
    ~ConnectionPool()=default;
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    static ConnectionPool&getInstance();
    void init(const std::string& connStr,int size);
    std::unique_ptr<soci::session>getConnection();
    void returnConnection(std::unique_ptr<soci::session> conn);
private:
    std::vector<std::unique_ptr<soci::session>>ConPool;
    std::mutex mtx;
    std::condition_variable cv;
    std::string connectionString;
    int poolSize;
};
class ConnectionWrapper {
public:
    ConnectionWrapper(ConnectionPool* pool,std::unique_ptr<soci::session>conn):pool(pool),connection(std::move(conn)){}
    ~ConnectionWrapper(){
        if(connection){
            pool->returnConnection(std::move(connection));
        }
    }
    soci::session*operator->(){
        return connection.get();
    }
    soci::session&operator*(){
        return *connection;
    }
private:
    ConnectionPool* pool;
    std::unique_ptr<soci::session>connection;
};