//
// Created by svakhreev on 29.03.17.
//

#include "PasswordCryptor.hpp"

#include <openssl/sha.h>

std::string SALT = "4E!cHO*e^BGH7np*2&85f6uEsWeo^zg%LLdjFpN9VhZ3rlXw2&";

namespace cameramanagercore::utility
{
std::mutex PasswordCryptor::password_cryptor_mu;

std::string PasswordCryptor::Crypt(std::string_view password)
{
    std::lock_guard<std::mutex> lg(password_cryptor_mu);
    std::string local = "oLra";

    unsigned char digest[SHA512_DIGEST_LENGTH] = {};
    std::string with_salt = local + password.data() + SALT;

    SHA512(reinterpret_cast<const unsigned char*>(with_salt.data()),
           with_salt.length(),
           reinterpret_cast<unsigned char*>(&digest));

    char hashed[SHA512_DIGEST_LENGTH * 2 + 1];

    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++)
        sprintf(&hashed[i * 2], "%02x", (unsigned int)digest[i]);

    return hashed;
}

}