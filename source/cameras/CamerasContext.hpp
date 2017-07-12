//
// Created by svakhreev on 08.02.17.
//

#ifndef CAMERAMANAGERCORE_CAMERASCONTEXT_HPP
#define CAMERAMANAGERCORE_CAMERASCONTEXT_HPP

#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <chrono>
#include <mutex>
#include <optional>
#include <string_view>
#include <variant>
#include <memory>
#include <numeric>
#include <limits>
#include <boost/format.hpp>
#include <onvif_wrapper/Onvif.hpp>
#include <range/v3/all.hpp>

#include "./interactions/Axis.hpp"
#include "./CameraContextVisitors.hpp"
#include "./parcers/JsonOnvifParcer.hpp"
#include "../model/Action.hpp"
#include "../model/Camera.hpp"
#include "../model/CameraLockFrame.hpp"
#include "../utility/LoggerSettings.hpp"
#include "../utility/VisitorHelper.hpp"
#include "../utility/RandomGenerator.hpp"
#include "../utility/TupleUtility.hpp"
#include "../utility/FuncUtility.hpp"
#include "../signals/SignalContext.hpp"

namespace std
{
template<> struct hash< std::shared_ptr<cameramanagercore::model::Camera> >
{
    size_t operator()(const std::shared_ptr<cameramanagercore::model::Camera>& camera_device) const
    {
        if (camera_device)
            return hash<int>{}(camera_device->id);
        return size_t {};
    }
};
}

namespace cameramanagercore::cameras
{

using onvifwrapper::Onvif;
using onvifwrapper::Result;
using cameramanagercore::cameras::interactions::Axis;
using cameramanagercore::model::Camera;
using cameramanagercore::model::Action;
using cameramanagercore::model::CameraLockFrame;
using namespace ranges;
using namespace cameramanagercore::signals;
using namespace cameramanagercore::utility;

template<typename SignalContext>
struct _CamerasContextFindedCamera;

template <typename SignalContext>
struct _ImplCamerasContext
{
    using CameraPtr = std::shared_ptr<Camera>;
    using InteractionT = std::variant<std::shared_ptr<Onvif>, std::shared_ptr<Axis>>;
    using ContainerT = std::unordered_map<CameraPtr, std::tuple<InteractionT, CameraLockFrame>>;
    using SignalContextPtr = std::shared_ptr<SignalContext>;

    friend struct _CamerasContextFindedCamera<SignalContext>;

    _ImplCamerasContext(SignalContextPtr signal_context)
        : _signal_context(signal_context),
          _lock_signal_generator(_signal_context->template AddSignalGenerator<CameraLockFrame>())
    {
        _camera_updater = [&](auto cd) mutable
        {
            switch (cd.status)
            {
                case DataStatus::Added:
                {
                    Add(cd.data);
                    break;
                }

                case DataStatus::Changed:
                {
                    std::unique_lock<std::mutex> ul(_mu_cameras);
                    auto ptr = v3::find_if(cameras_devices,
                                           [&](const auto& cam) { return cam.first->id == cd.data->id; });
                    if (ptr != cameras_devices.cend())
                        cameras_devices.erase(ptr);
                    ul.unlock();

                    Add(cd.data);
                    break;
                }

                case DataStatus::Removed:
                {
                    std::lock_guard<std::mutex> lg(_mu_cameras);
                    auto ptr = v3::find_if(cameras_devices,
                                           [&](const auto& cam) { return cam.first->id == cd.data->id; });
                    if (ptr != cameras_devices.cend())
                        cameras_devices.erase(ptr);
                    break;
                }
            }
        };

        _signal_context->template AddSubscriber<Camera>(_camera_updater);
    }

    _ImplCamerasContext(SignalContextPtr signal_context, std::vector<std::shared_ptr<Camera>> cameras)
        : _signal_context(signal_context),
          _lock_signal_generator(_signal_context->template AddSignalGenerator<CameraLockFrame>())
    {
        _camera_updater = [&](auto cd) mutable
        {
            switch (cd.status)
            {
                case DataStatus::Added:
                {
                    Add(cd.data);
                    break;
                }

                case DataStatus::Changed:
                {
                    std::unique_lock<std::mutex> ul(_mu_cameras);
                    auto ptr = v3::find_if(cameras_devices,
                                           [&](const auto& cam) { return cam.first->id == cd.data->id; });
                    if (ptr != cameras_devices.cend())
                        cameras_devices.erase(ptr);
                    ul.unlock();

                    Add(cd.data);
                    break;
                }

                case DataStatus::Removed:
                {
                    std::lock_guard<std::mutex> lg(_mu_cameras);
                    auto ptr = v3::find_if(cameras_devices,
                                           [&](const auto& cam) { return cam.first->id == cd.data->id; });
                    if (ptr != cameras_devices.cend())
                        cameras_devices.erase(ptr);
                    break;
                }
            }
        };

        _signal_context->template AddSubscriber<Camera>(_camera_updater);

        Fill(cameras);
    }

    ~_ImplCamerasContext()
    {
        _signal_context->RemoveSignalGenerator(_lock_signal_generator);
        _signal_context->template RemoveSubscriber<Camera>(_camera_updater);
    }

    void Fill(std::initializer_list<std::shared_ptr<Camera>> cameras) noexcept
    {
        for (const auto& camera: cameras)
            Add(camera);
    }

    void Fill(std::vector<std::shared_ptr<Camera>>& cameras) noexcept
    {
        for (const auto& camera: cameras)
            Add(camera);
    }

    void Add(const std::shared_ptr<Camera>& camera) noexcept
    {
        switch (camera->interaction_id)
        {
            case cameramanagercore::model::CameraInteraction::Onvif:
            {
                auto camera_frame = std::make_tuple(std::make_shared<Onvif>(camera->login,
                                                                  camera->password,
                                                                  camera->url), CameraLockFrame{ false,
                                                                                                 camera->id });
                {
                    std::lock_guard<std::mutex> lg(_mu_cameras);
                    cameras_devices.emplace(camera, std::move(camera_frame));
                }
                LOG(info) << boost::format("Added new onvif camera: %1%") % *camera;
                break;
            }
            case cameramanagercore::model::CameraInteraction::Axis:
            {
                auto camera_frame = std::make_tuple(std::make_shared<Axis>(camera->login,
                                                                 camera->password,
                                                                 camera->url), CameraLockFrame{ false,
                                                                                                camera->id });
                {
                    std::lock_guard<std::mutex> lg(_mu_cameras);
                    cameras_devices.emplace(camera, std::move(camera_frame));
                }
                LOG(info) << boost::format("Added new axis camera: %1%") % *camera;
                break;
            }
            default: break;
        }

    }

    void Clear() noexcept
    {
        std::lock_guard<std::mutex> lg(_mu_cameras);
        cameras_devices.clear();
    }

    std::optional<CameraLockFrame> Lock(std::shared_ptr<Camera>& camera, int user_id, std::chrono::seconds timeout) noexcept
    {
        using namespace std::chrono;

        std::unique_lock<std::mutex> ul(_mu_cameras);
        auto result = cameras_devices.find(camera);
        if (result == cameras_devices.cend())
        {
            LOG(error) << "Camera for lock not founded";
            return {};
        }

        LOG(info) << boost::format("Camera founded for lock: %1%") % *camera;
        auto& [interaction, lock] = result->second;
        ul.unlock();

        lock = CameraLockFrame { true,
                                 camera->id,
                                 user_id,
                                 CurrentTime(),
                                 static_cast<int>(timeout.count()),
                                 RandomGenerator::RandomString(50) };
        _lock_signal_generator->GenerateSignal(create_signal_data(DataStatus::Added,
                                                                  std::make_shared<CameraLockFrame>(lock)));
        return lock;
    }

    std::optional<CameraLockFrame> IsLocked(std::shared_ptr<Camera>& camera) noexcept
    {
        std::lock_guard<std::mutex> lg(_mu_cameras);
        using namespace std::chrono;

        auto result = cameras_devices.find(camera);
        if (result == cameras_devices.cend())
        {
            return {};
        }

        auto& [interaction, lock] = result->second;

        if (CurrentTime() - lock.locked_time >= lock.timeout)
            lock.locked = false;

        return lock;
    }

    std::optional<CameraLockFrame> Unlock(std::shared_ptr<Camera>& camera, std::string_view hash) noexcept
    {
        std::unique_lock<std::mutex> ul(_mu_cameras);
        using namespace std::chrono;

        auto result = cameras_devices.find(camera);
        if (result == cameras_devices.cend())
        {
            LOG(error) << "Camera for unlock not founded";
            return {};
        }

        LOG(info) << boost::format("Camera founded for unlock: %1%") % *camera;
        auto& [interaction, lock] = result->second;
        ul.unlock();

        if (lock.lock_hash == hash)
        {
            LOG(info) << boost::format("Camera unlocked: %1%") % *camera;
            lock.locked = false;
            _lock_signal_generator->GenerateSignal(create_signal_data(DataStatus::Added,
                                                                      std::make_shared<CameraLockFrame>(lock)));
        }
        else
        {
            LOG(info) << boost::format("Camera did not unlock because invalid hash: %1%") % *camera;
        }

        return lock;
    }

    void GenerateLockFramesSignals(DataStatus status) noexcept
    {
        std::unique_lock<std::mutex> ul(_mu_cameras);
        std::vector<std::tuple<InteractionT, CameraLockFrame>> values = cameras_devices | view::values;
        ul.unlock();

        for(auto&& frame: values)
        {
            auto& [interaction, lock] = frame;
            if (CurrentTime() - lock.locked_time >= lock.timeout)
                lock.locked = false;
            _lock_signal_generator->GenerateSignal(create_signal_data(status,
                                                                      std::make_shared<CameraLockFrame>(lock)));
        };
    }

    _CamerasContextFindedCamera<SignalContext> operator | (const std::shared_ptr<Camera>& camera) noexcept
    {
        std::lock_guard<std::mutex> lg(_mu_cameras);

        auto result = cameras_devices.find(camera);
        if (result != cameras_devices.cend())
        {
            LOG(trace) << boost::format("Camera founded for interaction: %1%") % *camera;
            return { _signal_context, *this, *result };
        }
        else
        {
            LOG(error) << "Camera for interaction not founded";
            return { _signal_context, *this, {} };
        }
    }

private:
    SignalContextPtr _signal_context;
    ContainerT cameras_devices;
    std::shared_ptr<SignalGenerator<CameraLockFrame>> _lock_signal_generator;
    std::function<void(SignalData<Camera>)> _camera_updater;
    mutable std::mutex _mu_cameras;

    template<typename Func>
    auto _InteractImpl(InteractionT& interaction, Func&& f) noexcept
    {
        return Visit(interaction, std::forward<std::remove_reference_t<Func>>(f));
    }

    int CurrentTime() const noexcept
    {
        using namespace std::chrono;
        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }
};

using CamerasContext = _ImplCamerasContext<SignalsContext>;

template<typename>
struct _CamerasContextFindedCamera
{
    template < typename Func, typename... OtherArgs >
    auto
    operator | (std::tuple< Func, /* calling functor */
                            std::string,   /* session_hash */
                            OtherArgs... /* other args */ > t)
    {
        using ReturnType = return_type_t< Func, Axis* >;

        if (!_camera_pair) return std::make_tuple(false, std::optional<ReturnType>{});

        auto [functor, session_hash] = take_first_n<2>(t);
        auto& [interaction, lock] = _camera_pair.value().second;
        cameramanagercore::model::CameraAction camera_action { false, _camera_pair.value().first->id, session_hash, 0, "",
                                                               RandomGenerator::RandomInt(0, std::numeric_limits<int32_t>::max()) };

        if constexpr (sizeof...(OtherArgs) == 0)
        {
            auto partial_apply = [_signal_context = this->_signal_context,
                                  functor = std::forward<std::remove_reference_t<Func>>(functor),
                                  camera_action]
                (auto&& interaction) { return functor(std::forward<std::remove_reference_t<decltype(interaction)>>(interaction),
                                                      _signal_context,
                                                      camera_action); };

            return lock.locked ?
                   std::make_tuple(true, std::optional<ReturnType>{})
                               :
                   std::make_tuple(false, _camera_context._InteractImpl(interaction, partial_apply));
        }
        else
        {
            auto partial_apply = [_signal_context = this->_signal_context,
                                  functor = std::forward<Func>(functor),
                                  camera_action,
                                  other = take_last_n<sizeof...(OtherArgs)>(t)]
                (auto&& interaction) { return functor(std::forward<std::remove_reference_t<decltype(interaction)>>(interaction),
                                                      _signal_context,
                                                      camera_action,
                                                      other); };

            return lock.locked ?
                   std::make_tuple(true, std::optional<ReturnType>{})
                               :
                   std::make_tuple(false, _camera_context._InteractImpl(interaction, partial_apply));
        }
    }


//    no lock
    template < typename Func, typename... OtherArgs >
    auto
    operator & (std::tuple< Func, /* calling functor */
                            std::string,   /* session_hash */
                            OtherArgs... /* other args */ > t)
    {
        using ReturnType = return_type_t< Func, Axis* >;
        if (!_camera_pair) return std::optional<ReturnType>{};

        auto [functor, session_hash] = take_first_n<2>(t);
        auto& [interaction, lock] = _camera_pair.value().second;
        cameramanagercore::model::CameraAction camera_action { false, _camera_pair.value().first->id, session_hash, 0, "",
                                                               RandomGenerator::RandomInt(0, std::numeric_limits<int32_t>::max()) };

        if constexpr (sizeof...(OtherArgs) == 0)
        {
            auto partial_apply = [_signal_context = this->_signal_context,
                                  functor = std::forward<Func>(functor),
                                  camera_action]
                (auto&& interaction) { return functor(std::forward<std::remove_reference_t<decltype(interaction)>>(interaction),
                                                      _signal_context,
                                                      camera_action); };

            return std::optional<ReturnType>{_camera_context._InteractImpl(interaction, partial_apply)};
        }
        else
        {
            auto partial_apply = [_signal_context = this->_signal_context,
                                  functor = std::forward<Func>(functor),
                                  camera_action,
                                  other = take_last_n<sizeof...(OtherArgs)>(t)]
                (auto&& interaction) { return functor(std::forward<std::remove_reference_t<decltype(interaction)>>(interaction),
                                                      _signal_context,
                                                      camera_action,
                                                      other); };

            return std::optional<ReturnType>{_camera_context._InteractImpl(interaction, partial_apply)};
        }
    }

//     no lock, action specialization, generate without action signal
    std::optional<Result> operator & (Action action)
    {
        if (!_camera_pair) return {};

        auto& [interaction, lock] = _camera_pair.value().second;
        return _camera_context._InteractImpl(interaction, [action] (auto&& interaction)
                                                          { return FromAction(interaction, action); });
    }

    SignalContextPtr _signal_context;
    CamerasContext& _camera_context;
    std::optional<typename CamerasContext::ContainerT::value_type> _camera_pair;
};

}


#endif //CAMERAMANAGERCORE_CAMERASCONTEXT_HPP
