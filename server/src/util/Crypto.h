#pragma once

#include <openssl/rand.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
class Crypto {
public:
    Crypto() = delete;
    static std::string generateSalt(size_t length = 16);
    static std::string hashPassword(const std::string& password, const std::string& salt);
};
