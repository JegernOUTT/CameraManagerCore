//
// Created by svakhreev on 09.03.17.
//

#ifndef CAMERAMANAGERCORE_RANDOMGENERATOR_HPP
#define CAMERAMANAGERCORE_RANDOMGENERATOR_HPP

#include <string>
#include <random>
#include <chrono>
#include <range/v3/all.hpp>

using namespace std::chrono;

struct RandomGenerator
{
    static int32_t RandomInt(int32_t min, int32_t max);
    static float RandomFloat(float min, float max);
    static std::string RandomString(size_t str_size = 50u);

    static std::default_random_engine generator;

    template<typename Duration>
    static Duration RandomDuration(int32_t min, int32_t max)
    {
        std::uniform_int_distribution<int32_t> distribution(min, max);
        return Duration{ distribution(generator) };
    }
};

#endif //CAMERAMANAGERCORE_RANDOMGENERATOR_HPP
