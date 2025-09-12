#include "ConnectionPool.h"
#include <soci/mysql/soci-mysql.h>
#include <iostream>

std::unique_ptr<ConnectionPool> ConnectionPool::instance = nullptr;
void ConnectionPool::initInstance(asio::io_context& ioc) {
    if (!instance) { 
        instance = std::unique_ptr<ConnectionPool>(new ConnectionPool(ioc));
    }
}

ConnectionPool& ConnectionPool::getInstance(){
    if (!instance) {
        throw std::runtime_error("ConnectionPool has not been initialized. Call initInstance first.");
    }
    return *instance;
}
void ConnectionPool::init(const std::string& connStr,int size){
    std::unique_lock<std::mutex> lock(mtx);
    if (!ConPool.empty()) {
        return;
    }
    connectionString=connStr;
    poolSize=size;
    for(int i=0;i<poolSize;++i){
        auto connection=std::make_unique<soci::session>(soci::mysql,connectionString);
        ConPool.push_back(std::move(connection));
    }
    std::cout << "Connection pool initialized with " << ConPool.size() << " connections." << std::endl;
}
std::unique_ptr<soci::session> ConnectionPool::getConnection(){
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock,[this]{ return !ConPool.empty(); });
    auto connection=std::move(ConPool.back());
    ConPool.pop_back();
    return connection;
}
void ConnectionPool::returnConnection(std::unique_ptr<soci::session> conn){
    std::unique_lock<std::mutex> lock(mtx);
    ConPool.push_back(std::move(conn));
    lock.unlock();
    cv.notify_one();
}
void ConnectionPool::replenishConnectionAsync(){
    asio::post(io_context, [this]() {
        try {
            auto new_conn = std::make_unique<soci::session>(soci::mysql, connectionString);
            returnConnection(std::move(new_conn));
            std::cout << "[ConnectionPool] A new connection has been replenished." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "[ConnectionPool] Failed to replenish connection: " << e.what() << std::endl;
        }
    });
}