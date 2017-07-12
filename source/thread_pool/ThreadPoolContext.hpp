//
// Created by svakhreev on 04.04.17.
//

#ifndef CAMERAMANAGERCORE_THREADPOOLCONTEXT_HPP
#define CAMERAMANAGERCORE_THREADPOOLCONTEXT_HPP

#include <unordered_map>
#include <memory>

namespace cameramanagercore::thread_pool
{

enum ThreadPoolType
{
    Common, Cameras
};

template<typename ThreadPool>
struct ThreadPoolContext
{
    std::unordered_map<ThreadPoolType, std::shared_ptr<ThreadPool>> pools;
};

}

#endif //CAMERAMANAGERCORE_THREADPOOLCONTEXT_HPP
