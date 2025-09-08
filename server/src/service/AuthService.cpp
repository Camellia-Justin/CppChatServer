#include "AuthService.h"

void AuthService::handleLogin(std::shared_ptr<Session> session, const chat::LoginRequest& loginRequest){
    chat::Envelope response_envelope;
    auto userOpt = userRepository->findByUsername(loginRequest.username());
    if(!userOpt) {
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("User not found.");
        session->send(response_envelope);
        return;
    }
    User user = *userOpt;
    if(user.getHashedPassword() != Crypto::hashPassword(loginRequest.password(), user.getSalt())){
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("Password incorrect.");
        session->send(response_envelope);
        return;
    }
    sessionManager->registerAuthenticatedSession(session, user.getId(), user.getUsername());
    
    auto* login_resp = response_envelope.mutable_login_response();
    login_resp->set_success(true);
    login_resp->set_user_id(std::to_string(user.getId()));
    login_resp->set_message("Login successful. Welcome, " + user.getUsername() + "!");
    session->send(response_envelope);
}
void AuthService::handleRegister(std::shared_ptr<Session> session, const chat::RegistrationRequest& registrationRequest){
    chat::Envelope response_envelope;
    if(userRepository->findByUsername(registrationRequest.username())){
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("Username already taken.");
        session->send(response_envelope);
        return;
    }
    auto salt = Crypto::generateSalt();
    auto hashedPassword = Crypto::hashPassword(registrationRequest.password(), salt);
    User newUser;
    newUser.setUsername(registrationRequest.username());
    newUser.setHashedPassword(hashedPassword);
    newUser.setSalt(salt);
    userRepository->addUser(newUser);   
    auto* reg_resp = response_envelope.mutable_registration_response();
    reg_resp->set_success(true);
    reg_resp->set_message("Registration successful. You can now log in, " + newUser.getUsername() + "!");
    session->send(response_envelope);
}
void AuthService::handleChangePassword(std::shared_ptr<Session> session, const chat::ChangePasswordRequest& changePasswordRequest){
    chat::Envelope  response_envelope;
    if(!session->isAuthenticated()){
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("Not authenticated.");
        session->send(response_envelope);
        return;
    }
    auto userOpt = userRepository->findByUsername(session->getUsername());
    if(!userOpt) {
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("User not found.");
        session->send(response_envelope);
        return;
    }
    User user = *userOpt;
    if(Crypto::hashPassword(changePasswordRequest.old_password(), user.getSalt()) != user.getHashedPassword()){
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("Old password is incorrect.");
        session->send(response_envelope);
        return;
    }
    if(changePasswordRequest.old_password().empty() || changePasswordRequest.new_password().empty()){
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("Old password and new password must be provided.");
        session->send(response_envelope);
        return;
    }
    if(changePasswordRequest.old_password() == changePasswordRequest.new_password()){
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("New password must be different from old password.");
        session->send(response_envelope);
        return;
    }
    user.setSalt(Crypto::generateSalt());
    user.setHashedPassword(Crypto::hashPassword(changePasswordRequest.new_password(), user.getSalt()));
    userRepository->updateUser(user);

    auto* change_resp = response_envelope.mutable_change_password_response();
    change_resp->set_success(true);
    change_resp->set_message("Password changed successfully.");
    session->send(response_envelope);
}
void AuthService::handleChangeUsername(std::shared_ptr<Session> session, const chat::ChangeUsernameRequest& changeUsernameRequest){
    chat::Envelope  response_envelope;
    if(!session->isAuthenticated()){
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("Not authenticated.");
        session->send(response_envelope);
        return;
    }
    auto userOpt = userRepository->findByUsername(session->getUsername());
    if(!userOpt) {
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("User not found.");
        session->send(response_envelope);
        return;
    }
    if(userRepository->findByUsername(changeUsernameRequest.new_username())){
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("Username already taken.");
        session->send(response_envelope);
        return;
    }
    User user = *userOpt;
    if(changeUsernameRequest.new_username().empty()){
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("New username must be provided.");
        session->send(response_envelope);
        return;
    }
    if(changeUsernameRequest.new_username() == user.getUsername()){
        auto* err_resp = response_envelope.mutable_error_response();
        err_resp->set_error_message("New username must be different from current username.");
        session->send(response_envelope);
        return;
    }
    user.setUsername(changeUsernameRequest.new_username());
    userRepository->updateUser(user);
    sessionManager->updateUsername(session, user.getUsername());

    auto* change_resp = response_envelope.mutable_change_username_response();
    change_resp->set_success(true);
    change_resp->set_message("Username changed successfully to " + changeUsernameRequest.new_username() + ".");
    session->send(response_envelope);
}