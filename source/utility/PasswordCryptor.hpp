//
// Created by svakhreev on 29.03.17.
//

#ifndef CAMERAMANAGERCORE_PASSWORDCRYPTOR_HPP
#define CAMERAMANAGERCORE_PASSWORDCRYPTOR_HPP

#include <string>
#include <mutex>
#include <string_view>

namespace cameramanagercore::utility
{

struct PasswordCryptor
{
    static std::string Crypt(std::string_view password);
    static std::mutex password_cryptor_mu;
};

}


#endif //CAMERAMANAGERCORE_PASSWORDCRYPTOR_HPP
