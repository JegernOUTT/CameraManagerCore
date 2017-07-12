//
// Created by svakhreev on 29.12.16.
//

#ifndef CAMERAMANAGERCORE_SECRETEKEYGENERATOR_HPP
#define CAMERAMANAGERCORE_SECRETEKEYGENERATOR_HPP

#include <string>

#include <openssl/rsa.h>
#include <openssl/pem.h>

#include "../utility/RandomGenerator.hpp"

namespace cameramanagercore::session
{

template<typename AlgorithmType>
struct SecreteKeyGenerator
{
    std::string operator()();
};

struct rsa_alg {};
struct random_alg {};

template<>
struct SecreteKeyGenerator<rsa_alg>
{
    std::string operator()()
    {
        RSA* rsa = RSA_new();
        BIGNUM* e = BN_new();
        BN_set_word(e, 3);

        if (RSA_generate_key_ex(rsa, 1024, e, nullptr))
        {
            BIO *bio = BIO_new(BIO_s_mem());
            PEM_write_bio_RSAPrivateKey(bio, rsa, nullptr, nullptr, 0, nullptr, nullptr);

            char* pem_key = nullptr;
            int keylen = 0;
            keylen = BIO_pending(bio);
            pem_key = static_cast<char*>(calloc(keylen + 1, 1));
            BIO_read(bio, pem_key, keylen);

            std::string result(pem_key);
            free(pem_key);
            BN_clear_free(e);
            RSA_free(rsa);
            BIO_free(bio);

            return result;
        }

        BN_clear_free(e);
        RSA_free(rsa);
        return {};
    }
};

template<>
struct SecreteKeyGenerator<random_alg>
{
    std::string operator()()
    {
        return RandomGenerator::RandomString(20);
    }
};

}


#endif //CAMERAMANAGERCORE_SECRETEKEYGENERATOR_HPP
