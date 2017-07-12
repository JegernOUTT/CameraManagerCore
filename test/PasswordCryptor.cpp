//
// Created by svakhreev on 29.03.17.
//

#include <iostream>
#include <unordered_map>
#include <string>

#include <bandit/bandit.h>

#include "../source/utility/PasswordCryptor.hpp"
#include "../source/utility/RandomGenerator.hpp"

using namespace bandit;
using namespace cameramanagercore::utility;

using std::unordered_map;
using std::string;

go_bandit([]{
    describe("Crypt / decrypt test", []{
        it("Crypt / decrypt test", [&]{
            for (size_t i = 0; i < 100; ++i)
            {
                unordered_map<string, string> passwords;

                for (size_t j = 0; j < 100; ++j)
                {
                    auto password = RandomGenerator::RandomString(100);
                    auto hashed = PasswordCryptor::Crypt(password);
                    passwords[password] = hashed;
                }

                for (auto&& [password, hash]: passwords)
                {
                    auto hashed = PasswordCryptor::Crypt(password);
                    AssertThat(hash, Is().EqualTo(hashed));
                }
            }
        });
    });
});