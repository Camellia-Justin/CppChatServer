#pragma once

#include <string>
#include "data/IUserRepository.h"
#include "core/SessionManager.h"
#include "session/Session.h"
#include "util/Crypto.h"
#include "chat.pb.h"
class AuthService {
public:
    AuthService(IUserRepository* userRepository, SessionManager* sessionManager): userRepository(userRepository), sessionManager(sessionManager) {}
    void handleLogin(std::shared_ptr<Session> session, const chat::LoginRequest& loginRequest);
    void handleRegister(std::shared_ptr<Session> session, const chat::RegistrationRequest& registrationRequest);
    void handleChangePassword(std::shared_ptr<Session> session, const chat::ChangePasswordRequest& changePasswordRequest);
    void handleChangeUsername(std::shared_ptr<Session> session, const chat::ChangeUsernameRequest& changeUsernameRequest);
private:
    IUserRepository* userRepository;
    SessionManager* sessionManager;
};
