//
// Created by svakhreev on 07.03.17.
//

#ifndef CAMERAMANAGERCORE_CAMERASTATUSPROCESSOR_HPP
#define CAMERAMANAGERCORE_CAMERASTATUSPROCESSOR_HPP

#include <memory>
#include <vector>
#include <mutex>
#include <onvif_wrapper/Onvif.hpp>
#include <boost/format.hpp>

#include "../CameraContextVisitors.hpp"
#include "../../signals/SignalContext.hpp"
#include "../../utility/LoggerSettings.hpp"
#include "../../thread_pool/Misc.hpp"

namespace cameramanagercore::model
{

struct Camera;
struct CameraStatus;

}

namespace cameramanagercore::cameras::processors
{

using namespace onvifwrapper;
using namespace cameramanagercore::signals;
using namespace cameramanagercore::model;
using namespace std::string_literals;

template < typename SignalContext,
           typename CameraContext,
           typename ModelContext,
           typename EventContext,
           typename ThreadPool >
class CameraStatusProcessor
{
    using CameraContextPtr = std::shared_ptr<CameraContext>;
    using ModelContextPtr  = std::shared_ptr<ModelContext>;
    using ThreadPoolPtr    = std::shared_ptr<ThreadPool>;
    using SignalContextPtr = std::shared_ptr<SignalContext>;
    using EventContextPtr  = std::shared_ptr<EventContext>;

public:
    CameraStatusProcessor(SignalContextPtr signal_context,
                          CameraContextPtr camera_context,
                          ModelContextPtr model_context,
                          EventContextPtr event_context,
                          ThreadPoolPtr thread_pool)
        : _signal_context(signal_context),
          _camera_context(camera_context),
          _model_context(model_context),
          _thread_pool(thread_pool),
          _camera_updater([&] (auto cd) mutable
                          {
                              std::lock_guard<std::mutex> lg(_mu_cameras);
                              _cameras = _model_context->GetCameras();
                          }),
          _signal(_signal_context->template AddSignalGenerator<CameraStatus>()),
          _cameras(model_context->GetCameras())
    {
        _signal_context->template AddSubscriber<Camera>(_camera_updater);
    }

    ~CameraStatusProcessor()
    {
        _signal_context->template RemoveSubscriber<Camera>(_camera_updater);
        _signal_context->template RemoveSignalGenerator<CameraStatus>(_signal);
    }

    void operator()() noexcept
    {
        std::unique_lock<std::mutex> ul(_mu_cameras);
        std::vector<std::shared_ptr<Camera>> cameras = _cameras;
        ul.unlock();

        for (const auto& camera: cameras)
        {
            auto get_camera_status = [&] ()
            {
                auto maybe = ((*_camera_context) | camera) & std::make_tuple(GetStatus, ""s);
                using ReturnType = typename decltype(maybe)::value_type;
                return maybe.value_or(ReturnType{});
            };

            auto maybe_from_pool =
            _thread_pool  | boost::packaged_task<std::optional<std::tuple<Result, PTZStatus>>> { get_camera_status }
                          | cameramanagercore::thread_pool::execute;

            if (maybe_from_pool && maybe_from_pool.value())
            {
                auto [status, result] = maybe_from_pool.value().value();
                _signal->GenerateSignal(create_signal_data(DataStatus::Added,
                                                           std::make_shared<CameraStatus>(CameraStatus{ camera->id, result })));
                LOG(info) << boost::format("Get status ok with camera: %1%") % *camera;
            }
            else
            {
                LOG(error) << boost::format("Error while get status request with camera: %1%") % *camera;
            }
        };
    }

private:
    SignalContextPtr _signal_context;
    CameraContextPtr _camera_context;
    ModelContextPtr _model_context;
    ThreadPoolPtr _thread_pool;

    mutable std::mutex _mu_cameras;
    std::function<void(SignalData<Camera>)> _camera_updater;
    std::shared_ptr<SignalGenerator<CameraStatus>> _signal;

    std::vector<std::shared_ptr<Camera>> _cameras;
};

}

#endif //CAMERAMANAGERCORE_CAMERASTATUSPROCESSOR_HPP
