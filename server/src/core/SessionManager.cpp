#include "session/Session.h"
#include "SessionManager.h"
#include <iostream>
SessionManager::SessionManager(std::recursive_mutex& mtx) : mtx(mtx) {}
void SessionManager::add(std::shared_ptr<Session> s){
    std::lock_guard<std::recursive_mutex> lock(mtx);
    sessions.insert(s);
}
void SessionManager::remove(std::shared_ptr<Session> s){
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (s->isAuthenticated()) {
        sessionsByUserId.erase(s->getUserId());
        sessionsByUsername.erase(s->getUsername());
    }
    sessions.erase(s);
}
void SessionManager::updateUsername(std::shared_ptr<Session> s, const std::string& newUsername){
    std::lock_guard<std::recursive_mutex> lock(mtx);
    sessionsByUsername.erase(s->getUsername());
    s->setUsername(newUsername);
    sessionsByUsername[newUsername] = s;
}
void SessionManager::registerAuthenticatedSession(std::shared_ptr<Session> s, long long userId, const std::string& username){
    std::lock_guard<std::recursive_mutex> lock(mtx);
    sessionsByUserId[userId] = s;
    sessionsByUsername[username] = s;
    s->setAuthenticated(userId, username);
    std::cout << "User " << username << " (ID: " << userId << ") authenticated and registered." << std::endl;
}
std::shared_ptr<Session> SessionManager::findByUsername(const std::string& username){
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto it = sessionsByUsername.find(username);
    if(it == sessionsByUsername.end()){
        std::cout << "No session found for username: " << username << std::endl;
        return nullptr;
    }
    return it->second;
}
std::shared_ptr<Session> SessionManager::findByUserId(long long userId){
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto it = sessionsByUserId.find(userId);
    if(it == sessionsByUserId.end()){
        std::cout << "No session found for user ID: " << userId << std::endl;
        return nullptr;
    }
    return it->second;
}
void SessionManager::broadcast(const chat::Envelope& envelop){
    for(const auto& session : sessions){
        if(session->isAuthenticated()){
            session->send(envelop);
        }
    }
}