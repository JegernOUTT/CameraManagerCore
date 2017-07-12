//
// Created by svakhreev on 10.01.17.
//

#ifndef CAMERAMANAGERCORE_SIGNAL_CONTEXT_HPP
#define CAMERAMANAGERCORE_SIGNAL_CONTEXT_HPP

#include <list>
#include <memory>
#include <tuple>
#include <mutex>
#include <functional>
#include <type_traits>
#include <wigwag/token_pool.hpp>

#include "signal_inheritance/SignalGenerator.hpp"

namespace cameramanagercore::model
{
struct User;
struct Camera;
struct Event;
struct Permission;
struct Settings;
struct UserCameras;
struct UserPermissions;
struct CameraLockFrame;
struct CameraAction;
struct CameraStatus;
struct CameraInIdle;
struct Sessions;
struct EventArchiveSettings;
}

namespace cameramanagercore::signals
{

template<typename... Types>
struct SignalContext
{
    template<typename T>
    std::shared_ptr<SignalGenerator<T>> AddSignalGenerator() noexcept
    {
        LOG(trace) << "Generator added";

        std::unique_lock<std::mutex> ul(_mutex_generators);
        std::get<std::list<std::shared_ptr<SignalGenerator<T>>>>(_generators).emplace_back(SignalGenerator<T>::Create());
        auto signal_generator = std::get<std::list<std::shared_ptr<SignalGenerator<T>>>>(_generators).back();
        ul.unlock();

        tokens += signal_generator->s.connect([this] (typename SignalGenerator<T>::Data&& item)
                                              { ProcessAllSubscribers<T>(std::forward<typename SignalGenerator<T>::Data>(item)); },
                                              wigwag::handler_attributes::suppress_populator);
        return signal_generator;
    }
    template<typename T>
    void RemoveSignalGenerator(const std::shared_ptr<SignalGenerator<T>>& signal_generator) noexcept
    {
        LOG(trace) << "Remove generator";

        std::lock_guard<std::mutex> lg(_mutex_generators);
        std::get<std::list<std::shared_ptr<SignalGenerator<T>>>>(_generators).remove(signal_generator);
    }
    template<typename... Args>
    void RemoveSignalGenerators() noexcept
    {
        LOG(trace) << "Remove generators";

        std::lock_guard<std::mutex> lg(_mutex_generators);
        (std::get<std::list<std::shared_ptr<SignalGenerator<Args>>>>(_generators).clear(), ...);
    }
    void RemoveSignalGenerators() noexcept
    {
        LOG(trace) << "Remove all generators";

        std::unique_lock<std::mutex> ul(_mutex_generators);
        std::apply([](auto&&... lists) { (lists.clear(), ...); }, _generators);
        ul.unlock();

        tokens.release();
    }
    template<typename T>
    size_t SignalGeneratorCount() const noexcept
    {
        std::lock_guard<std::mutex> lg(_mutex_generators);
        return std::get<std::list<std::shared_ptr<SignalGenerator<T>>>>(_generators).size();
    }

    template<typename T>
    void AddSubscriber(std::function<void(SignalData<T>)> f) noexcept
    {
        LOG(trace) << "Add subscriber for signal";

        std::lock_guard<std::mutex> lg(_mutex_subscribers);
        std::get<std::list<std::function<void(SignalData<T>)>>>(_subscribers).emplace_back(f);
    }
    template<typename T>
    void RemoveSubscriber(const std::function<void(SignalData<T>)>& subscriber) noexcept
    {
        LOG(trace) << "Remove subscriber";

        std::lock_guard<std::mutex> lg(_mutex_subscribers);
        std::get<std::list<std::function<void(SignalData<T>)>>>(_subscribers).remove(subscriber);
    }
    template<typename... Args>
    void RemoveSubscribers() noexcept
    {
        LOG(trace) << "Remove subscribers";

        std::lock_guard<std::mutex> lg(_mutex_subscribers);
        (std::get<std::list<std::function<void(SignalData<Args>)>>>(_subscribers).clear(), ...);
    }
    void RemoveSubscribers() noexcept
    {
        LOG(trace) << "Remove subscribers";

        std::lock_guard<std::mutex> lg(_mutex_subscribers);
        std::apply([](auto&&... lists) { (lists.clear(), ...); }, _subscribers);
    }
    template<typename T>
    size_t SubscribersCount() const
    {
        std::lock_guard<std::mutex> lg(_mutex_subscribers);
        return std::get<std::list<std::function<void(SignalData<T>)>>>(_subscribers).size();
    }

private:
    std::tuple<std::list<std::shared_ptr<SignalGenerator<Types>>>...> _generators;

    wigwag::token_pool tokens;
    std::tuple<std::list<std::function<void(SignalData<Types>)>>...> _subscribers;
    mutable std::mutex _mutex_generators;
    mutable std::mutex _mutex_subscribers;

    template<typename T>
    void ProcessAllSubscribers(typename SignalGenerator<T>::Data data)
    {
        std::unique_lock<std::mutex> ul(_mutex_subscribers);
        std::list<std::function<void(SignalData<T>)>> subscriber_list
            = std::get<std::list<std::function<void(SignalData<T>)>>>(_subscribers);
        ul.unlock();

        for (const auto& func: subscriber_list)
        {
            func(data);
        }
    }
};


using SignalsContext = SignalContext < cameramanagercore::model::User,
                                       cameramanagercore::model::Camera,
                                       cameramanagercore::model::Event,
                                       cameramanagercore::model::Permission,
                                       cameramanagercore::model::Settings,
                                       cameramanagercore::model::UserCameras,
                                       cameramanagercore::model::UserPermissions,
                                       cameramanagercore::model::CameraLockFrame,
                                       cameramanagercore::model::CameraAction,
                                       cameramanagercore::model::CameraInIdle,
                                       cameramanagercore::model::CameraStatus,
                                       cameramanagercore::model::Sessions,
                                       cameramanagercore::model::EventArchiveSettings >;
}

#endif //CAMERAMANAGERCORE_SIGNAL_CONTEXT_HPP
