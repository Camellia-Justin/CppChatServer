#pragma once

#include<asio.hpp>
#include <soci/soci.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
class ConnectionPool {
public:
    ~ConnectionPool()=default;
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    static void initInstance(asio::io_context& ioc);
    static ConnectionPool& getInstance();
    void init(const std::string& connStr, int size);
    std::unique_ptr<soci::session> getConnection();
    void returnConnection(std::unique_ptr<soci::session> conn);
    void replenishConnectionAsync();
private:
    ConnectionPool(asio::io_context& ioc) : io_context(ioc) {}
    static std::unique_ptr<ConnectionPool> instance;
    std::vector<std::unique_ptr<soci::session>>ConPool;
    std::mutex mtx;
    asio::io_context& io_context;
    std::condition_variable cv;
    std::string connectionString;
    int poolSize;
    
};
class ConnectionWrapper {
public:
    ConnectionWrapper(ConnectionPool* pool,std::unique_ptr<soci::session>conn):pool(pool),connection(std::move(conn)),is_valid(true){}
    ~ConnectionWrapper(){
        if(connection){
			if (is_valid)
            {
                pool->returnConnection(std::move(connection));
            }
            else {
				pool->replenishConnectionAsync();
            }
        }
    }
    void markAsInvalid() {
        is_valid = false;
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
    bool is_valid;
};