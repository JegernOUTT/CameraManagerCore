//}

// Created by svakhreev on 07.03.17.
//

#ifndef CAMERAMANAGERCORE_CAMERAIDLEACTIONPROCESSOR_HPP
#define CAMERAMANAGERCORE_CAMERAIDLEACTIONPROCESSOR_HPP

#include <memory>
#include <vector>
#include <chrono>
#include <mutex>
#include <unordered_map>

#include <onvif_wrapper/Onvif.hpp>
#include <boost/format.hpp>
#include <boost/thread/future.hpp>
#include <boost/circular_buffer.hpp>
#include <range/v3/all.hpp>
#include <nlohmann/json.hpp>

#include "../CameraContextVisitors.hpp"
#include "../../signals/SignalContext.hpp"
#include "../../model/Camera.hpp"
#include "../../model/CameraInformation.hpp"
#include "../../thread_pool/Misc.hpp"
#include "../../utility/LoggerSettings.hpp"

namespace cameramanagercore::cameras::processors
{

using namespace ranges;
using namespace onvifwrapper;
using namespace cameramanagercore::signals;
using namespace cameramanagercore::model;

template < typename SignalContext,
           typename CameraContext,
           typename ModelContext,
           typename EventContext,
           typename ThreadPool >
class IdleActionProcessor
{
    using CameraContextPtr  = std::shared_ptr<CameraContext>;
    using ModelContextPtr   = std::shared_ptr<ModelContext>;
    using ThreadPoolPtr     = std::shared_ptr<ThreadPool>;
    using CameraActionFrame = std::tuple<bool, boost::circular_buffer<CameraAction>>;
    using SignalContextPtr  = std::shared_ptr<SignalContext>;
    using EventContextPtr   = std::shared_ptr<EventContext>;

public:
    IdleActionProcessor(SignalContextPtr signal_context,
                        CameraContextPtr camera_context,
                        ModelContextPtr model_context,
                        EventContextPtr event_context,
                        ThreadPoolPtr thread_pool)
        : _signal_context(signal_context),
          _camera_context(camera_context),
          _model_context(model_context),
          _thread_pool(thread_pool),
          _idle_generator(signal_context->template AddSignalGenerator<CameraInIdle>())
    {
        _camera_updater = [&](auto cd) mutable
        {
            switch (cd.status)
            {
                case DataStatus::Added:
                {
                    std::lock_guard<std::mutex> lg(_mu_cameras);
                    _cameras_actions.emplace(cd.data, std::make_tuple(false, boost::circular_buffer<CameraAction> {10}));
                    break;
                }

                case DataStatus::Changed:
                {
                    std::lock_guard<std::mutex> lg(_mu_cameras);
                    auto ptr = v3::find_if(_cameras_actions,
                                           [&](const auto& cam) { return cam.first->id == cd.data->id; });
                    if (ptr != _cameras_actions.cend())
                        _cameras_actions.erase(ptr);
                    _cameras_actions.emplace(cd.data, std::make_tuple(false, boost::circular_buffer<CameraAction> {10}));
                    break;
                }

                case DataStatus::Removed:
                {
                    std::lock_guard<std::mutex> lg(_mu_cameras);
                    auto ptr = v3::find_if(_cameras_actions,
                                           [&](const auto& cam) { return cam.first->id == cd.data->id; });
                    if (ptr != _cameras_actions.cend())
                        _cameras_actions.erase(ptr);
                    break;
                }
            }
        };

        _action_updater = [&](auto cd) mutable
        {
            std::lock_guard<std::mutex> lg(_mu_cameras);
            auto ptr = v3::find_if(_cameras_actions,
                                   [&](const auto& cam) { return cam.first->id == cd.data->camera_id; });
            if (ptr == _cameras_actions.cend())
                LOG(error) << boost::format("Camera not founded for action: %1%") % *cd.data;

            auto& [in_idle, buffer] = ptr->second;
            in_idle = false;
            buffer.push_back(*cd.data);
            LOG(info) << boost::format("Action (%1%) recorded for camera: %2%") % *cd.data % *ptr->first;
        };

        FillCamerasActions();
        _signal_context->template AddSubscriber<Camera>(_camera_updater);
        _signal_context->template AddSubscriber<CameraAction>(_action_updater);
    }

    ~IdleActionProcessor()
    {
        _signal_context->template RemoveSubscriber<Camera>(_camera_updater);
        _signal_context->template RemoveSubscriber<CameraAction>(_action_updater);
        _signal_context->template RemoveSignalGenerator<CameraInIdle>(_idle_generator);
    }

    void operator()() noexcept
    {
        std::unique_lock<std::mutex> ul(_mu_cameras);
        std::unordered_map<std::shared_ptr<Camera>, CameraActionFrame> cameras_actions = _cameras_actions;
        ul.unlock();

        GenerateSignals(cameras_actions);

        for (auto& cam_act: cameras_actions)
        {
            auto& [camera, frame] = cam_act;
            auto& [in_idle, buffer] = frame;
            if (in_idle) continue;

            const auto& action = buffer.back();
            if (Now() - action.time <= camera->idle_timeout_sec) continue;

            Action camera_action;

            try
            {
                camera_action = nlohmann::json::parse(camera->on_idle_action);
            }
            catch (...)
            {
                LOG(error) << boost::format("Error while action parsing from camera: %1%") % *camera;
                continue;
            }

            auto action_func = [&, camera = decltype(camera) { camera }] ()
            {
                return ((*_camera_context) | camera) & camera_action;
            };

            // TODO: async execute without wait
            _thread_pool | boost::packaged_task<std::optional<Result>> { action_func }
                         | cameramanagercore::thread_pool::execute;

            std::unique_lock<std::mutex> ul(_mu_cameras);
            auto& [idle, b] = _cameras_actions[camera];
            idle = true;
            ul.unlock();

            LOG(info) << boost::format("Camera going to idle position: %1%") % *camera;
        };
    }

private:
    void FillCamerasActions()
    {
        _cameras_actions.clear();
        auto cameras = _model_context->GetCameras();

        for (const auto& camera: cameras)
        {
            _cameras_actions.emplace(camera, std::make_tuple(false, boost::circular_buffer<CameraAction> {10}));
        }
    }

    int Now()
    {
        using namespace std::chrono;
        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }

    void GenerateSignals(const std::unordered_map<std::shared_ptr<Camera>, CameraActionFrame>& cameras)
    {
        for (const auto& cam_act: cameras)
        {
            auto& [camera, frame] = cam_act;
            auto& [in_idle, buffer] = frame;
            _idle_generator->GenerateSignal(create_signal_data(DataStatus::Added,
                                                               std::make_shared<CameraInIdle>(
                                                                   CameraInIdle { camera->id, in_idle })));
        }
    }

private:
    SignalContextPtr _signal_context;
    CameraContextPtr _camera_context;
    ModelContextPtr  _model_context;
    ThreadPoolPtr    _thread_pool;

    mutable std::mutex _mu_cameras;
    std::function<void(SignalData<Camera>)> _camera_updater;
    std::function<void(SignalData<CameraAction>)> _action_updater;
    std::shared_ptr<SignalGenerator<CameraInIdle>> _idle_generator;

    std::unordered_map<std::shared_ptr<Camera>, CameraActionFrame> _cameras_actions;
};

}

#endif //CAMERAMANAGERCORE_CAMERAIDLEACTIONPROCESSOR_HPP
