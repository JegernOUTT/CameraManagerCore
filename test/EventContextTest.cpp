//
// Created by svakhreev on 06.03.17.
//

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <future>
#include <random>
#include <iostream>
#include <type_traits>
#include <sstream>
#include <bandit/bandit.h>
#include <range/v3/all.hpp>

#include "../source/cameras/parcers/JsonOnvifParcer.hpp"
#include "../source/events/EventContext.hpp"
#include "../source/signals/SignalContext.hpp"

using namespace bandit;
using namespace ranges;
using namespace cameramanagercore::model;
using namespace cameramanagercore::events;
using namespace cameramanagercore::signals;

using std::move;
using std::make_unique;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;
using std::map;
using std::is_same_v;

go_bandit([] {
    describe("Event context tests", [] {
        auto signals_context = make_shared<SignalsContext>();
        shared_ptr<Event> event1, event2, event3;

        bandit::before_each([&] {
            event1 = make_shared<Event>(Event { "as2df3a4f", "Event 1", "Event 1 description", 1 });
            event2 = make_shared<Event>(Event { "x5cg24356", "Event 2", "Event 2 description", 2 });
            event3 = make_shared<Event>(Event { "12241b5sd", "Event 3", "Event 3 description", 2 });
        });

        it("Simple test", [&] {
            auto sort_comparer = [] (const Event& lhs, const Event& rhs) { return lhs.name < rhs.name; };

            vector<Event> all_events {*event1, *event3, *event2};
            all_events |= action::sort(sort_comparer);

            EventContext context { signals_context, {event1, event2, event3} };
            vector<shared_ptr<Event>> events = context.GetAllEvents();
            vector<Event> new_events = events | view::transform([] (auto&& event) { return *event; });
            new_events |= action::sort(sort_comparer);

            AssertThat(context.GetAllEvents().size(), Is().EqualTo(3));
            AssertThat(new_events, Is().EqualToContainer(all_events));

            auto maybe_event = context.Create(Event { "1rsdt23", "Event 4", "Event 4 description", 1 }, "some_session_hash");
            AssertThat(static_cast<bool>(maybe_event), Is().True());
            AssertThat(maybe_event.value()->created_session_hash, Is().EqualTo("some_session_hash"));
            auto [is_changed, changed_event] = context.Change(maybe_event.value(), EventStatus::Processed, "some_session_hash");
            AssertThat(is_changed, Is().True());
            AssertThat(changed_event->processed, Is().True());
            AssertThat(changed_event->processed_time, Is().GreaterThan(0));
            AssertThat(context.GetStatus(changed_event), Is().EqualTo(EventStatus::Processed));

            auto [_is_changed, _changed_event] = context.Change(maybe_event.value(), EventStatus::Acknowledged, "some_session_hash");
            AssertThat(_changed_event->acknowledged, Is().True());
            AssertThat(_changed_event->acknowledged_time, Is().GreaterThan(0));
            AssertThat(_changed_event->acknowledge_session_hash, Is().EqualTo("some_session_hash"));
            AssertThat(context.GetStatus(_changed_event), Is().EqualTo(EventStatus::ProcessedAndAcknowledged));
        });
    });
});