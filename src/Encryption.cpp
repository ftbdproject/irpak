// src/Encryption.cpp
#include "Encryption.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>

std::vector<uint8_t> Encryption::encrypt(const std::vector<uint8_t>& data,
                                       const std::string& password,
                                       const uint8_t* salt,
                                       const uint8_t* iv) {
    uint8_t key[32];
    deriveKey(password, salt, key);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create encryption context");
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }

    std::vector<uint8_t> encrypted;
    encrypted.resize(data.size() + EVP_MAX_BLOCK_LENGTH);
    int outlen1 = 0, outlen2 = 0;

    if (EVP_EncryptUpdate(ctx, encrypted.data(), &outlen1,
                         data.data(), static_cast<int>(data.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed during encryption");
    }

    if (EVP_EncryptFinal_ex(ctx, encrypted.data() + outlen1, &outlen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize encryption");
    }

    unsigned char tag[16];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to get encryption tag");
    }

    EVP_CIPHER_CTX_free(ctx);

    encrypted.resize(outlen1 + outlen2);
    encrypted.insert(encrypted.end(), tag, tag + 16);

    return encrypted;
}

std::vector<uint8_t> Encryption::decrypt(const std::vector<uint8_t>& data,
                                       const std::string& password,
                                       const uint8_t* salt,
                                       const uint8_t* iv) {
    if (data.size() < 16) {
        throw std::runtime_error("Invalid encrypted data size");
    }

    std::vector<uint8_t> encryptedData(data.begin(), data.end() - 16);
    std::vector<uint8_t> tag(data.end() - 16, data.end());

    uint8_t key[32];
    deriveKey(password, salt, key);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create decryption context");
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }

    std::vector<uint8_t> decrypted;
    decrypted.resize(encryptedData.size());
    int outlen1 = 0, outlen2 = 0;

    if (EVP_DecryptUpdate(ctx, decrypted.data(), &outlen1,
                         encryptedData.data(), 
                         static_cast<int>(encryptedData.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed during decryption");
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to set decryption tag");
    }

    if (EVP_DecryptFinal_ex(ctx, decrypted.data() + outlen1, &outlen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption failed - invalid password or corrupted data");
    }

    EVP_CIPHER_CTX_free(ctx);
    decrypted.resize(outlen1 + outlen2);
    
    return decrypted;
}

void Encryption::generateSaltAndIV(uint8_t* salt, uint8_t* iv) {
    if (RAND_bytes(salt, 32) != 1 || RAND_bytes(iv, 16) != 1) {
        throw std::runtime_error("Failed to generate cryptographic random bytes");
    }
}

void Encryption::deriveKey(const std::string& password,
                         const uint8_t* salt,
                         uint8_t* key) {
    if (PKCS5_PBKDF2_HMAC(password.c_str(),
                          static_cast<int>(password.length()),
                          salt, 32,
                          10000,
                          EVP_sha256(),
                          32, key) != 1) {
        throw std::runtime_error("Failed to derive key from password");
    }
}