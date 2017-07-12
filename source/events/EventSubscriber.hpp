//
// Created by svakhreev on 07.03.17.
//

#ifndef CAMERAMANAGERCORE_EVENTSUBSCRIBER_HPP
#define CAMERAMANAGERCORE_EVENTSUBSCRIBER_HPP

#include <memory>
#include <functional>
#include <tuple>
#include <mutex>
#include <list>
#include <optional>

#include <boost/format.hpp>
#include <boost/thread/future.hpp>
#include <nlohmann/json.hpp>

#include "../model/Action.hpp"
#include "../model/Event.hpp"
#include "../cameras/CamerasContext.hpp"
#include "../utility/LoggerSettings.hpp"
#include "../signals/SignalContext.hpp"
#include "../thread_pool/ExecutionWrapper.hpp"
#include "../thread_pool/pool/ThreadPool.hpp"

namespace cameramanagercore::events
{

using namespace cameramanagercore::model;
using namespace cameramanagercore::signals;
using namespace cameramanagercore::thread_pool;

template < typename SignalContext,
           typename EventContext,
           typename CameraContext,
           typename ModelContext,
           typename Pool >
struct EventSubscriber
{
    using SignalContextPtr = std::shared_ptr<SignalContext>;
    using EventContextPtr  = std::shared_ptr<EventContext>;
    using CameraContextPtr = std::shared_ptr<CameraContext>;
    using ModelContextPtr  = std::shared_ptr<ModelContext>;
    using PoolPtr          = std::shared_ptr<Pool>;

    EventSubscriber(SignalContextPtr signal_context,
                    EventContextPtr  event_context,
                    CameraContextPtr camera_context,
                    PoolPtr          camera_interaction_pool,
                    ModelContextPtr  model_context)
        : _signal_context(signal_context),
          _event_context(event_context),
          _camera_context(camera_context),
          _pool(camera_interaction_pool),
          _model_context(model_context),
          _callback([this](SignalData<Event> d)
                    {
                        this->ProcessEvent(d);
                    })
    {
        _signal_context->template AddSubscriber<Event>(_callback);
    }

    ~EventSubscriber()
    {
        _signal_context->template RemoveSubscriber<Event>(_callback);
    }

private:
    void ProcessEvent(SignalData<Event> data) noexcept
    {
        auto events = _event_context->GetEvents();
        auto camera_events = events | view::filter([&] (auto&& event) { return event->camera_id == data.data->camera_id; });

        if (data.status == DataStatus::Added)
        {
            auto filtered = camera_events | view::filter([&] (auto&& e)
                                                         {
                                                             auto status = _event_context->GetStatus(e);
                                                             return status == EventStatus::Initial
                                                                    || status == EventStatus::Processed;
                                                         });
            if (std::vector<std::shared_ptr<Event>> {filtered}.size() <= 1) Process(data);
        }
        else if (data.status == DataStatus::Changed)
        {
            auto status = _event_context->GetStatus(data.data);
            switch (status)
            {
                case EventStatus::Acknowledged:
                case EventStatus::ProcessedAndAcknowledged:
                {
                    auto initial_with_less_time = camera_events
                                                  | view::filter([&] (auto&& e)
                                                                 {
                                                                     return _event_context->GetStatus(e)
                                                                            == EventStatus::Initial;
                                                                 });
                    std::vector<std::shared_ptr<Event>> filtered_vector = initial_with_less_time;

                    filtered_vector |= action::sort([] (auto&& e1, auto&& e2)
                                                    {
                                                        return e1->created_time < e2->created_time;
                                                    });
                    auto one = filtered_vector | view::take(1);
                    if (std::vector<std::shared_ptr<Event>>{ one }.size() > 0)
                        Process(std::vector<std::shared_ptr<Event>>{ one }.front());
                };
                break;

                default: break;
            }
        }
    }

    void Process(SignalData<Event> data) noexcept
    {
        ProcessImpl(data.data);
    }

    void Process(std::shared_ptr<Event> event) noexcept
    {
        ProcessImpl(event);
    }

    void ProcessImpl(std::shared_ptr<Event> event) noexcept
    {
        auto maybe_camera =_model_context->FindCameraById(event->camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Camera id (%1%) not found for event: %2%")
                          % event->camera_id
                          % *event;
            return;
        }

        auto camera = maybe_camera.value();
        Action action;
        try
        {
            action = nlohmann::json::parse(event->on_create_action);
        }
        catch (...)
        {
            LOG(error) << boost::format("Action parsing error in event: %1%")
                          % *event;
            return;
        }

        auto maybe_result =
            _pool | boost::packaged_task<std::optional<Result>> { [&] { return (*_camera_context | camera) & action; } }
                  | execute;
        if (!maybe_result || !maybe_result.value())
        {
            LOG(error) << boost::format("Error in on_create event action %1% processing with camera %2%")
                          % action
                          % *camera;
            return;
        }

        auto result = maybe_result.value().value();
        if (result.is_error)
        {
            LOG(error) << boost::format("Error in on_create event action %1% while camera interaction: %2%")
                          % action
                          % nlohmann::json(result);
            return;
        }

        auto [ok, updated_event] = _event_context->Change(event, EventStatus::Processed, "");
        if (!ok)
        {
            LOG(error) << boost::format("Event %1% status did not updated. Some undefined state in event")
                          % *event;
            return;
        }

        LOG(info) << boost::format("On_create event action successfull, updated event: %1%")
                     % *updated_event;
    }

    SignalContextPtr _signal_context;
    EventContextPtr  _event_context;
    CameraContextPtr _camera_context;
    PoolPtr          _pool;
    ModelContextPtr  _model_context;
    std::function<void(SignalData<Event>)> _callback;
};

}

#endif //CAMERAMANAGERCORE_EVENTSUBSCRIBER_HPP
