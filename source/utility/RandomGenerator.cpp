//
// Created by svakhreev on 09.03.17.
//

#include "RandomGenerator.hpp"

using namespace std;
using namespace std::chrono;
using namespace ranges;

default_random_engine RandomGenerator::generator(high_resolution_clock::now().time_since_epoch().count());

int32_t RandomGenerator::RandomInt(int32_t min, int32_t max)
{
    uniform_int_distribution<int32_t> distribution(min, max);
    return distribution(generator);
}

float RandomGenerator::RandomFloat(float min, float max)
{
    uniform_real_distribution<float> distribution(min, max);
    return distribution(generator);
}

string RandomGenerator::RandomString(size_t str_size)
{
    uniform_int_distribution<int32_t> choise_distribution(0, 1);
    uniform_int_distribution<int32_t> char_distribution('a', 'z');
    uniform_int_distribution<int32_t> digit_distribution('0', '9');
    return string { view::generate_n([&]
                                     {
                                         switch(choise_distribution(generator))
                                         {
                                             case 0:  return char_distribution(generator);
                                             default: return digit_distribution(generator);
                                         }
                                     }, str_size) };
}