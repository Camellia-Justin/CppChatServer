#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_set>
class Session;
namespace chat{
    class Envelope;
}

class SessionManager {
public:
    explicit SessionManager(std::recursive_mutex& mtx);
    ~SessionManager() = default;
    void add(std::shared_ptr<Session> s);
    void remove(std::shared_ptr<Session> s);
    void updateUsername(std::shared_ptr<Session> s, const std::string& newUsername);
    void registerAuthenticatedSession(std::shared_ptr<Session> s, long long userId, const std::string& username);
    std::shared_ptr<Session> findByUsername(const std::string& username);
    std::shared_ptr<Session> findByUserId(long long userId);
    void broadcast(const chat::Envelope& envelope);

private:
    std::recursive_mutex& mtx;
    std::unordered_set<std::shared_ptr<Session>> sessions;
    std::unordered_map<long long, std::shared_ptr<Session>> sessionsByUserId;
    std::unordered_map<std::string, std::shared_ptr<Session>> sessionsByUsername;
};