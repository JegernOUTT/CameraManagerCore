//
// Created by svakhreev on 07.03.17.
//

#ifndef CAMERAMANAGERCORE_CAMERASTATUSUPDATER_HPP
#define CAMERAMANAGERCORE_CAMERASTATUSUPDATER_HPP

#include <memory>
#include <thread>
#include <atomic>
#include <tuple>
#include <chrono>
#include <mutex>

#include <boost/thread.hpp>
#include <onvif_wrapper/Onvif.hpp>

#include "../signals/SignalContext.hpp"

namespace cameramanagercore::cameras
{

using namespace onvifwrapper;
using namespace cameramanagercore::signals;
using namespace cameramanagercore::model;

template < typename SignalContext,
           typename CameraContext,
           typename ModelContext,
           typename EventContext,
           typename ThreadPool,
           typename... Processors >
struct CameraLooper
{
    using CameraContextPtr = std::shared_ptr<CameraContext>;
    using ModelContextPtr = std::shared_ptr<ModelContext>;
    using EventContextPtr = std::shared_ptr<EventContext>;
    using ThreadPoolPtr = std::shared_ptr<ThreadPool>;
    using ProcessorsTuple = std::tuple<std::shared_ptr<Processors>...>;
    using SignalContextPtr = std::shared_ptr<SignalContext>;

    CameraLooper(std::chrono::milliseconds pause_timeout,
                 SignalContextPtr signal_context,
                 CameraContextPtr camera_context,
                 ModelContextPtr model_context,
                 EventContextPtr event_context,
                 ThreadPoolPtr pool)
        : _timeout(pause_timeout),
          _signal_context(signal_context),
          _camera_context(camera_context),
          _model_context(model_context),
          _event_context(event_context),
          _pool(pool),
          _th(std::unique_ptr<std::thread>{}),
          _cancel(true)
    {
        Run();
    }

    ~CameraLooper()
    {
        if (!_cancel) Stop();
    }

    void Run()
    {
        if (!_cancel) return;

        _cancel = false;
        CreateProcessors();
        _th = std::make_unique<std::thread>([&]() { RunImpl(); });
    }

    void Stop()
    {
        if (_cancel) return;

        _cancel = true;
        _th->join();
        DestroyProcessors();
    }

private:

    void CreateProcessors()
    {
        _processors = std::make_tuple(std::make_shared<Processors>(_signal_context, _camera_context, _model_context,
                                                                   _event_context, _pool)...);
    }

    void DestroyProcessors()
    {
        _processors = {};
    }

    void RunImpl()
    {
        while (!_cancel)
        {
            std::apply([](auto&&... processors) { (((*processors)()), ...); },
                       _processors);
            std::this_thread::sleep_for(_timeout);
        }
    }

private:
    std::chrono::milliseconds _timeout;
    SignalContextPtr _signal_context;
    CameraContextPtr _camera_context;
    ModelContextPtr _model_context;
    EventContextPtr _event_context;
    ThreadPoolPtr _pool;

    ProcessorsTuple _processors;

    std::unique_ptr<std::thread> _th;
    std::atomic_bool _cancel;
};

}

#endif //CAMERAMANAGERCORE_CAMERASTATUSUPDATER_HPP
