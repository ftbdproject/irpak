// include/Encryption.hpp
#pragma once
#include <vector>
#include <string>

class Encryption
{
public:
    static std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data,
                                      const std::string& password,
                                      const uint8_t* salt,
                                      const uint8_t* iv);

    static std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data,
                                      const std::string& password,
                                      const uint8_t* salt,
                                      const uint8_t* iv);

    static void generateSaltAndIV(uint8_t* salt, uint8_t* iv);
    static void deriveKey(const std::string& password,
                         const uint8_t* salt,
                         uint8_t* key);
};