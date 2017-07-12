//
// Created by svakhreev on 10.01.17.
//

#include <memory>
#include <sstream>
#include <thread>
#include <chrono>
#include <tuple>
#include <vector>
#include <iostream>
#include <tuple>

#include <bandit/bandit.h>
#include <boost/thread.hpp>
#include <range/v3/all.hpp>

#include "../source/utility/RandomGenerator.hpp"
#include "../source/thread_pool/pool/ThreadPool.hpp"
#include "../source/thread_pool/ExecutionWrapper.hpp"

using namespace bandit;
using namespace ranges;
using namespace std::chrono;
using namespace std::this_thread;
using namespace cameramanagercore::thread_pool;

using ThreadPoolStd = ThreadPool<>;


go_bandit([] {
    describe("Simple test", [] {
        it("Test", [&]{

            auto func_1 = [&]
            {
                std::thread {[] { sleep_for(RandomGenerator::RandomDuration<seconds>(1, 3)); }}.join();
                return 45;
            };

            auto func_2 = [&]
            {
                std::thread {[] { sleep_for(RandomGenerator::RandomDuration<seconds>(1, 3)); }}.join();
                return 42;
            };


            ThreadPoolStd pool;

            auto result1 = pool
                           | boost::packaged_task<int>{func_1}
                           | set_error_if<int>([](int item) { return item > 42; })
                           | execute;
            auto result2 = pool
                           | boost::packaged_task<int>{func_2}
                           | set_error_if<int>([](int item) { return item > 42; })
                           | execute;

            AssertThat(static_cast<bool>(result1), Is().False());
            AssertThat(result2.value(), Is().EqualTo(42));
        });


        it("Concurrency test", [&]{

            auto func_1 = [&]
            {
                std::thread {[] { sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 20)); }}.join();
                return 45;
            };

            auto func_2 = [&]
            {
                std::thread {[] { sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 20)); }}.join();
                return 42;
            };

            ThreadPoolStd pool;

            std::vector<boost::thread> v;
            v.reserve(12);
            for (auto repeat: view::ints | view::take(12))
            {
                v.emplace_back(boost::thread{ [&]
                                              {
                                                  for (auto internal_repeat: view::ints | view::take(1000))
                                                  {
                                                      sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 20));
                                                      auto result1 = pool
                                                                     | boost::packaged_task<int>{func_1}
                                                                     | set_error_if<int>([](int item) { return item > 42; })
                                                                     | execute;
                                                      sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 20));
                                                      auto result2 = pool
                                                                     | boost::packaged_task<int>{func_2}
                                                                     | set_error_if<int>([](int item) { return item > 42; })
                                                                     | execute;
                                                      sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 20));

                                                      AssertThat(static_cast<bool>(result1), Is().False());
                                                      AssertThat(result2.value(), Is().EqualTo(42));
                                                  }
                                              }});
            }

            for (auto& t: v)
                t.join();
        });
    });

});