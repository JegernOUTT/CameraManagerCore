//
// Created by svakhreev on 29.12.16.
//

#ifndef CAMERAMANAGERCORE_SESSION_HPP
#define CAMERAMANAGERCORE_SESSION_HPP

#include <memory>
#include <string>
#include <chrono>

#include <boost/format.hpp>

#include "SecreteKeyGenerator.hpp"
#include "../utility/LoggerSettings.hpp"

namespace cameramanagercore::session
{

enum class SessionStatus
{
    Living, Expired
};

template < typename SecreteKeyGenerator, typename Duration, typename UserPtr >
struct Session
{
    Session(Duration duration, UserPtr user);
    void Touch();
    SessionStatus Status();
    UserPtr GetUser() const;
};

template<typename Duration, typename UserPtr>
struct Session<SecreteKeyGenerator<random_alg>, Duration, UserPtr>
{
    Session(Duration duration, UserPtr user)
        : _key(SecreteKeyGenerator<random_alg>{}()),
          _expired_duration(duration),
          _user(user),
          _last_interaction(time_point_cast<Duration>(std::chrono::system_clock::now()))
    {
        LOG(info) << (boost::format("New session created. Key: %1%. User: %2%") % _key % *_user).str();
    }

    void Touch()
    {
        _last_interaction = time_point_cast<Duration>(std::chrono::system_clock::now());
        LOG(info) << (boost::format("On session %1% new interaction (User: %2%)") % _key % *_user).str();
    }

    SessionStatus Status() const
    {
        auto now = time_point_cast<Duration>(std::chrono::system_clock::now());
        return now - _last_interaction > _expired_duration ? SessionStatus::Expired
                                                           : SessionStatus::Living;
    }

    UserPtr GetUser() const { return _user; };
    const std::string& Secrete() { return _key; }

private:
    std::string _key;
    Duration _expired_duration;
    UserPtr _user;
    std::chrono::time_point<std::chrono::system_clock, Duration> _last_interaction;
};

template < typename Duration, typename UserPtr >
auto make_session(Duration duration, UserPtr user)
{
    return std::make_shared<Session<SecreteKeyGenerator<random_alg>, Duration, UserPtr>>(duration, user);
}

}

#endif //CAMERAMANAGERCORE_SESSION_HPP
