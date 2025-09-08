#pragma once

#include <string>
#include "chat.pb.h"
#include "session/Session.h"
#include "core/SessionManager.h"
#include "data/IUserRepository.h"
#include "util/Crypto.h"
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
