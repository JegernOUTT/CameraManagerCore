//
// Created by svakhreev on 29.12.16.
//

#ifndef CAMERAMANAGERCORE_SESSIONCONTEXT_HPP
#define CAMERAMANAGERCORE_SESSIONCONTEXT_HPP

#include <list>
#include <vector>
#include <algorithm>
#include <chrono>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <optional>

#include <range/v3/all.hpp>

#include "./Session.hpp"
#include "../model/Sessions.hpp"

namespace cameramanagercore::session
{

using namespace ranges;
using cameramanagercore::model::Sessions;

enum class SessionClearPolicy
{
    Lazy, Async
};

template < typename SessionPtr,
           typename Duration,
           SessionClearPolicy Policy,
           typename ModelContext >
struct SessionContext;

template < typename SessionPtr,
           typename Duration,
           typename ModelContext >
struct SessionContext<SessionPtr, Duration, SessionClearPolicy::Lazy, ModelContext>
{
    SessionContext(std::shared_ptr<ModelContext> model_context)
        : _model_context(model_context)
    {}

    void AddSession(SessionPtr session) noexcept
    {
        std::unique_lock<std::mutex> ul(_mu);
        _sessions.emplace_back(session);
        ul.unlock();

        _model_context->Add(std::make_shared<Sessions>(Sessions{ session->Secrete(),
                                                            session->GetUser()->id,
                                                            std::chrono::duration_cast<milliseconds>(
                                                                system_clock::now().time_since_epoch()).count(),
                                                            0}));
    }

    std::list<SessionPtr> GetSessions() noexcept
    {
        std::lock_guard<std::mutex> lg(_mu);
        _sessions |=
            action::remove_if([](auto& session){ return session->Status() == SessionStatus::Expired; });
        return _sessions;
    }

    std::optional<SessionPtr> FindSession(std::string secrete) const noexcept
    {
        std::lock_guard<std::mutex> lg(_mu);
        auto it = std::find_if(_sessions.cbegin(),
                          _sessions.cend(),
                          [&secrete](auto& session) { return session->Secrete() == secrete; });

        if (it != _sessions.cend()) return *it;
        return {};
    }

    template<typename UserPtr>
    std::optional<SessionPtr> FindSession(UserPtr user) const noexcept
    {
        std::lock_guard<std::mutex> lg(_mu);
        auto it = std::find_if(_sessions.cbegin(),
                          _sessions.cend(),
                          [&user](auto& session) { return session->GetUser() == user; });

        if (it != _sessions.cend()) return *it;
        return {};
    }

    void RemoveSession(std::string secrete) const noexcept
    {
        std::unique_lock<std::mutex> ul(_mu);
        _sessions |= action::remove_if([&secrete](auto& session) { return session->Secrete() == secrete; });
        ul.unlock();

        auto maybe_session = _model_context->FindSessionByHash(secrete);
        if (maybe_session)
        {
            auto new_session = std::make_shared<Sessions>(maybe_session.value());
            new_session->destruction_time = std::chrono::duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            _model_context->Change(maybe_session.value(), new_session);
        }
    }

private:
    std::shared_ptr<ModelContext> _model_context;
    mutable std::mutex _mu;
    std::list<SessionPtr> _sessions;
};

template < typename SessionPtr,
           typename Duration,
           typename ModelContext >
struct SessionContext<SessionPtr, Duration, SessionClearPolicy::Async, ModelContext>
{
    SessionContext(std::shared_ptr<ModelContext> model_context)
        : _model_context(model_context),
          _async_update_duration(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::seconds(5))),
          _async_th(nullptr),
          _running(false)
    {
        _async_th = std::make_unique<std::thread>(&SessionContext::UpdateSessions, this);
    }

    SessionContext(std::shared_ptr<ModelContext> model_context, Duration duration)
        : _model_context(model_context),
          _async_update_duration(duration),
          _async_th(nullptr),
          _running(false)
    {
        _async_th = std::make_unique<std::thread>(&SessionContext::UpdateSessions, this);
    }

    ~SessionContext()
    {
        if (_async_th == nullptr) return;

        _running = false;
        _async_th->join();
    }

    void AddSession(SessionPtr session) noexcept
    {
        using namespace std::chrono;
        std::unique_lock<std::mutex> ul(_mu);
        _sessions.emplace_back(session);
        ul.unlock();

        _model_context->Add(std::make_shared<Sessions>(Sessions{ session->Secrete(),
                                                            session->GetUser()->id,
                                                            std::chrono::duration_cast<milliseconds>(
                                                                system_clock::now().time_since_epoch()).count(),
                                                            0}));
    }

    std::list<SessionPtr> GetSessions() noexcept
    {
        std::lock_guard<std::mutex> lg(_mu);
        return _sessions;
    }

    std::optional<SessionPtr> FindSession(std::string secrete) const noexcept
    {
        std::lock_guard<std::mutex> lg(_mu);

        auto it = std::find_if(_sessions.cbegin(),
                               _sessions.cend(),
                               [&secrete](auto& session) { return session->Secrete() == secrete; });

        if (it != _sessions.cend()) return *it;
        return {};
    }

    template<typename UserPtr>
    std::optional<SessionPtr> FindSession(UserPtr user) const noexcept
    {
        std::lock_guard<std::mutex> lg(_mu);

        auto it = std::find_if(_sessions.cbegin(),
                               _sessions.cend(),
                               [&user](auto& session) { return session->GetUser() == user; });

        if (it != _sessions.cend()) return *it;
        return {};
    }

    void RemoveSession(std::string secrete) noexcept
    {
        std::unique_lock<std::mutex> ul(_mu);
        _sessions |= action::remove_if([&secrete](auto& session) { return session->Secrete() == secrete; });
        ul.unlock();

        auto maybe_session = _model_context->FindSessionByHash(secrete);
        if (maybe_session)
        {
            auto new_session = std::make_shared<Sessions>(*maybe_session.value());
            new_session->destruction_time = std::chrono::duration_cast<milliseconds>(
                system_clock::now().time_since_epoch()).count();
            _model_context->Change(maybe_session.value(), new_session);
        }
    }

private:
    std::shared_ptr<ModelContext> _model_context;
    Duration _async_update_duration;
    std::list<SessionPtr> _sessions;

private:
    std::unique_ptr<std::thread> _async_th;
    mutable std::mutex _mu;
    std::atomic_bool _running;

    void UpdateSessions()
    {
        _running = true;
        const auto is_expired = [](auto& session){ return session->Status() == SessionStatus::Expired; };

        while (_running)
        {
            std::unique_lock<std::mutex> ul(_mu);
            std::vector<SessionPtr> removed = _sessions | view::filter(is_expired);
            _sessions |= action::remove_if(is_expired);
            ul.unlock();

            for (auto& session: removed)
            {
                auto secrete = session->Secrete();
                auto maybe_session = _model_context->FindSessionByHash(secrete);
                if (maybe_session)
                {
                    auto new_session = std::make_shared<Sessions>(*maybe_session.value());
                    new_session->destruction_time = std::chrono::duration_cast<milliseconds>(
                        system_clock::now().time_since_epoch()).count();
                    _model_context->Change(maybe_session.value(), new_session);
                }
            }

            std::this_thread::sleep_for(_async_update_duration);
        }
    }
};

}

#endif //CAMERAMANAGERCORE_SESSIONCONTEXT_HPP
