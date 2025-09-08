#include "Crypto.h"
std::string Crypto::generateSalt(size_t length) {
    std::vector<unsigned char> buffer(length);
    if (RAND_bytes(buffer.data(), length) != 1) {
        throw std::runtime_error("Failed to generate random salt.");
    }
    std::stringstream ss;
    for (unsigned char c : buffer) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
    return ss.str();
}
std::string Crypto::hashPassword(const std::string& password, const std::string& salt) {
    std::string to_hash = password + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, to_hash.c_str(), to_hash.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}