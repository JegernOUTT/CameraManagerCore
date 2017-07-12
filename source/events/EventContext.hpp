//
// Created by svakhreev on 06.03.17.
//

#ifndef CAMERAMANAGERCORE_EVENTCONTEXT_HPP
#define CAMERAMANAGERCORE_EVENTCONTEXT_HPP

#include <algorithm>
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>
#include <tuple>
#include <unordered_map>
#include <string_view>
#include <optional>

#include <boost/sml.hpp>
#include <boost/format.hpp>

#include "../model/Event.hpp"
#include "../model/detail/ContextMisc.hpp"
#include "../utility/LoggerSettings.hpp"
#include "../signals/SignalContext.hpp"

namespace cameramanagercore::events
{

using namespace std::chrono_literals;
using namespace cameramanagercore::signals;
using namespace cameramanagercore::model;

enum class OperationStatus;

struct created   { std::shared_ptr<Event> event; std::string_view session_hash; };
struct processed { std::shared_ptr<Event> event; };
struct ack       { std::shared_ptr<Event> event; std::string_view session_hash; };
struct archive   { std::shared_ptr<Event> event; };

struct notice_state
{
    auto operator()() noexcept
    {
        namespace sml = boost::sml;

        const auto initial_f = [](auto event)
        {
            LOG(info) << boost::format("Event %1% going INITIAL state") % *event.event;
            event.event->created_session_hash = event.session_hash.data();
            event.event->acknowledged = false;
            event.event->processed = false;
            event.event->acknowledged_time = 0l;
            event.event->processed_time = 0l;
        };

        const auto acknowledged_f = [](auto event)
        {
            LOG(info) << boost::format("Event %1% going ACKNOWLEDGED state") % *event.event;
            event.event->acknowledged = true;
            event.event->acknowledge_session_hash = event.session_hash.data();
            event.event->acknowledged_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        };

        const auto processed_f = [](auto event)
        {
            LOG(info) << boost::format("Event %1% going PROCESSED state") % *event.event;
            event.event->processed = true;
            event.event->processed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        };

        return sml::make_transition_table(
            *sml::state<class initial>    + sml::event<created>    / initial_f        = sml::state<class created>,
            sml::state<class initial>     + sml::event<archive>                       = sml::state<class archived>,
            sml::state<class created>     + sml::event<processed>  / processed_f      = sml::state<class processed>,
            sml::state<class created>     + sml::event<ack>        / acknowledged_f   = sml::state<class ack>,
            sml::state<class processed>   + sml::event<ack>        / acknowledged_f   = sml::state<class ack>,
            sml::state<class ack>                                                     = sml::X
        );
    }
};

template <typename SignalContext>
class EventContextImpl
{
public:
    using EventPtr = std::shared_ptr<Event>;
    using EventWPtr = std::weak_ptr<Event>;
    template<typename T>
    using SignalGeneratorT = std::shared_ptr<SignalGenerator<T>>;
    using SignalContextPtr = std::shared_ptr<SignalContext>;


    EventContextImpl(SignalContextPtr signal_context, std::vector<EventPtr> events)
        : _signal_context(signal_context),
          _signal_generator(_signal_context->template AddSignalGenerator<Event>())
    {
        for (auto&& event: events)
        {
            boost::sml::sm<notice_state> sm;
            sm.process_event(archive{ event });
            _events.emplace(event, std::move(sm));
        }
    }

    ~EventContextImpl()
    {
        _signal_context->RemoveSignalGenerator(_signal_generator);
    }

    std::optional<EventPtr>
    GetEvent(std::string_view event_hash) const noexcept
    {
        std::lock_guard<std::mutex> lg(_mu_main);
        auto finded = std::find_if(_events.cbegin(), _events.cend(),
                                   [&] (auto&& event) { return event.first->hash == event_hash; });
        return finded != _events.cend() ? finded->first : std::optional<EventPtr>{};
    }

    std::vector<EventPtr>
    GetEvents() const noexcept
    {
        namespace sml = boost::sml;

        std::unique_lock<std::mutex> ul(_mu_main);

        std::vector<EventPtr> filtered_events;
        std::for_each(_events.cbegin(), _events.cend(),
                      [&filtered_events] (auto&& e)
                      {
                          if (!std::get<1>(e).is(sml::state<class archived>))
                              filtered_events.emplace_back(std::get<0>(e));
                      });
        ul.unlock();

        std::sort(filtered_events.begin(),
                  filtered_events.end(),
                  [] (auto&& e1, auto&& e2) { return e1->created_time < e2->created_time; });
        return filtered_events;
    }

    std::vector<EventPtr>
    GetEvents(size_t count) const noexcept
    {
        namespace sml = boost::sml;

        std::unique_lock<std::mutex> ul(_mu_main);

        std::vector<EventPtr> filtered_events;
        std::for_each(_events.cbegin(), _events.cend(),
                      [&filtered_events] (auto&& e)
                      {
                          if (!std::get<1>(e).is(sml::state<class archived>))
                              filtered_events.emplace_back(std::get<0>(e));
                      });
        ul.unlock();

        std::sort(filtered_events.begin(),
                  filtered_events.end(),
                  [] (auto&& e1, auto&& e2) { return e1->created_time < e2->created_time; });
        return std::vector<EventPtr>(filtered_events.begin(), filtered_events.begin() + count);
    }

    std::vector<EventPtr>
    GetArchivedEvents() const noexcept
    {
        namespace sml = boost::sml;

        std::unique_lock<std::mutex> ul(_mu_main);

        std::vector<EventPtr> filtered_events;
        std::for_each(_events.cbegin(), _events.cend(),
                      [&filtered_events] (auto&& e)
                      {
                          if (std::get<1>(e).is(sml::state<class archived>))
                              filtered_events.emplace_back(std::get<0>(e));
                      });
        ul.unlock();

        std::sort(filtered_events.begin(),
                  filtered_events.end(),
                  [] (auto&& e1, auto&& e2) { return e1->created_time < e2->created_time; });
        return filtered_events;
    }

    std::vector<EventPtr>
    GetArchivedEvents(size_t count) const noexcept
    {
        namespace sml = boost::sml;

        std::unique_lock<std::mutex> ul(_mu_main);

        std::vector<EventPtr> filtered_events;
        std::for_each(_events.cbegin(), _events.cend(),
                      [&filtered_events] (auto&& e)
                      {
                          if (std::get<1>(e).is(sml::state<class archived>))
                              filtered_events.emplace_back(std::get<0>(e));
                      });
        ul.unlock();

        std::sort(filtered_events.begin(),
                  filtered_events.end(),
                  [] (auto&& e1, auto&& e2) { return e1->created_time < e2->created_time; });
        return std::vector<EventPtr>(filtered_events.begin(), filtered_events.begin() + count);
    }

    std::vector<EventPtr>
    GetAllEvents() const noexcept
    {
        std::unique_lock<std::mutex> ul(_mu_main);
        std::vector<EventPtr> events;
        std::transform(_events.cbegin(), _events.cend(), std::back_inserter(events),
                       [] (auto&& e) { return std::get<0>(e); });
        ul.unlock();

        std::sort(events.begin(),
                  events.end(),
                  [] (auto&& e1, auto&& e2) { return e1->created_time < e2->created_time; });
        return events;
    }

    std::vector<EventPtr>
    GetAllEvents(size_t count) const noexcept
    {
        std::unique_lock<std::mutex> ul(_mu_main);
        std::vector<EventPtr> events;
        std::transform(_events.cbegin(), _events.cend(), std::back_inserter(events),
                       [] (auto&& e) { return std::get<0>(e); });
        ul.unlock();


        std::sort(events.begin(),
                  events.end(),
                  [] (auto&& e1, auto&& e2) { return e1->created_time < e2->created_time; });
        return std::vector<EventPtr>(events.begin(), events.begin() + count);
    }

    std::optional<EventPtr>
    Create(const Event& event,
           std::string_view session_hash) noexcept
    {
        std::unique_lock<std::mutex> ul(_mu_main);

        if (ExistByValue(event))
        {
            LOG(error) << boost::format("Event already exist: %1%") % event;
            return {};
        }

        auto event_ptr = std::make_shared<Event>(event);

        LOG(info) << boost::format("New event created: %1%") % *event_ptr;
        boost::sml::sm<notice_state> sm;
        sm.process_event(created{ event_ptr, session_hash });
        _events.emplace(event_ptr, std::move(sm));
        ul.unlock();

        _signal_generator->GenerateSignal(create_signal_data(DataStatus::Added, event_ptr));
        return event_ptr;
    }

    std::optional<EventPtr>
    FindEvent(std::string_view hash) noexcept
    {
        std::unique_lock<std::mutex> ul(_mu_main);
        auto finded = std::find_if(_events.cbegin(), _events.cend(),
                                   [&](auto&& e) { return e.first->hash == hash; });
        if (finded == _events.cend()) return {};
        return finded->first;
    }

    std::tuple<bool, EventPtr>
    Change(const EventPtr& _event,
           EventStatus new_status,
           std::string_view session_hash) noexcept
    {
        namespace sml = boost::sml;

        std::unique_lock<std::mutex> ul(_mu_main);
        auto finded = _events.find(_event);
        if (finded == _events.cend())
        {
            LOG(error) << boost::format("Event not found for change: %1%") % *_event;
            return std::make_tuple(false, EventPtr{});
        }
        auto& [event, sm] = *finded;
        ul.unlock();

        switch(new_status)
        {
            case EventStatus::Processed:
            {
                auto status = GetStatus(_event);
                if (status == EventStatus::Processed
                    || status == EventStatus::ProcessedAndAcknowledged) return std::make_tuple(false, event);
                sm.process_event(processed{ event });
                if (sm.is(sml::state<class processed>)) _signal_generator->GenerateSignal(create_signal_data(DataStatus::Changed, event));
                return sm.is(sml::state<class processed>) ? std::make_tuple(true, event) : std::make_tuple(false, event);
            }

            case EventStatus::Acknowledged:
            {
                auto status = GetStatus(_event);
                if (status == EventStatus::Acknowledged
                    || status == EventStatus::ProcessedAndAcknowledged) return std::make_tuple(false, event);
                sm.process_event(ack{ event, session_hash });
                if (sm.is(sml::X)) _signal_generator->GenerateSignal(create_signal_data(DataStatus::Changed, event));
                return sm.is(sml::X) ? std::make_tuple(true, event) : std::make_tuple(false, event);
            }

            case EventStatus::HasVideo:
            {
                event->has_video = true;
                _signal_generator->GenerateSignal(create_signal_data(DataStatus::Changed, event));
                return std::make_tuple(true, event);
            }

            default:
                return std::make_tuple(false, event);
        }
    };

    bool RemoveEvent(const EventPtr& _event) noexcept
    {
        std::unique_lock<std::mutex> ul(_mu_main);
        auto count = _events.erase(_event);
        if (count == 0)
        {
            LOG(error) << boost::format("Event not found for removing: %1%") % *_event;
            return false;
        }
        ul.unlock();

        _signal_generator->GenerateSignal(create_signal_data(DataStatus::Removed, _event));
        LOG(info) << boost::format("Event removed: %1%") % *_event;
        return true;
    }

    EventStatus GetStatus(const EventPtr& event) const noexcept
    {
        if (!event->processed && !event->acknowledged)
        {
            return EventStatus::Initial;
        }
        else if (event->processed && !event->acknowledged)
        {
            return EventStatus::Processed;
        }
        else if (!event->processed && event->acknowledged)
        {
            return EventStatus::Acknowledged;
        }
        else if (event->processed && event->acknowledged)
        {
            return EventStatus::ProcessedAndAcknowledged;
        }

        return {};
    }

private:
    bool ExistByValue(const Event& event) const
    {
        return std::find_if(_events.cbegin(), _events.cend(),
                            [&](auto&& e) { return *(e.first) == event; }) != _events.cend();
    }

private:
    SignalContextPtr _signal_context;
    std::unordered_map<EventPtr, boost::sml::sm<notice_state>> _events;
    SignalGeneratorT<Event> _signal_generator;
    mutable std::mutex _mu_main;
};

using EventContext = EventContextImpl<SignalsContext>;

}

#endif //CAMERAMANAGERCORE_EVENTCONTEXT_HPP
