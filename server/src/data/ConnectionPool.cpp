#include "ConnectionPool.h"
#include <soci/mysql/soci-mysql.h>
#include <iostream>

ConnectionPool& ConnectionPool::getInstance(){
    static ConnectionPool instance;
    return instance;
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