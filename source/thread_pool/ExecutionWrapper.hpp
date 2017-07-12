//
// Created by svakhreev on 10.01.17.
//

#ifndef CAMERAMANAGERCORE_EXECUTIONWRAPPER_HPP
#define CAMERAMANAGERCORE_EXECUTIONWRAPPER_HPP

#include <memory>
#include <optional>
#include <boost/thread.hpp>

#include "Misc.hpp"

namespace cameramanagercore::thread_pool
{

template < typename InputType >
struct set_error_handler { std::function<void(std::optional<InputType>)> handler; };

template < typename InputType >
struct set_conditional { std::function<bool(InputType)> handler; };

template < typename InputType >
set_conditional<InputType> set_error_if(std::function<bool(InputType)> handler)
{
    return { handler };
}

template < typename InputType >
set_error_handler<InputType> on_error_do(std::function<void(std::optional<InputType>)> handler)
{
    return { handler };
}

template < typename ReturnType >
struct ThreadPoolWrapper
{
    using error_handler = set_error_handler<ReturnType>;
    using conditional_handler = set_conditional<ReturnType>;

    boost::unique_future<ReturnType> f;
    error_handler _error_handler = { [](std::optional<ReturnType>) {} };
    conditional_handler _conditional_handler = { [](ReturnType)->bool { return false; } };

    template < typename T, typename R >
    R operator|(T param);

    ThreadPoolWrapper& operator|(error_handler handler)
    {
        _error_handler = handler;
        return *this;
    }

    ThreadPoolWrapper& operator|(conditional_handler handler)
    {
        _conditional_handler = handler;
        return *this;
    }

    std::optional<ReturnType> operator|(exec_type)
    {
        try
        {
            auto value = f.get();
            if (!_conditional_handler.handler(value))
            {
                return value;
            }
            else
            {
                _error_handler.handler(value);
                return {};
            }
        }
        catch (...)
        {
            _error_handler.handler({});
            return {};
        }
    }
};

template < typename ThreadPool, typename ReturnType >
ThreadPoolWrapper<ReturnType> operator|(ThreadPool& pool,
                                        boost::packaged_task<ReturnType>&& handler)
{
    auto f = handler.get_future();
    pool.post(handler);
    return { std::move(f) };
}

template < typename ThreadPool, typename ReturnType >
ThreadPoolWrapper<ReturnType> operator|(std::shared_ptr<ThreadPool>& pool,
                                        boost::packaged_task<ReturnType>&& handler)
{
    auto f = handler.get_future();
    pool->post(handler);
    return { std::move(f) };
}

}

#endif //CAMERAMANAGERCORE_EXECUTIONWRAPPER_HPP
