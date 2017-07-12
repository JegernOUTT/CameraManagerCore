//
// Created by svakhreev on 11.01.17.
//

#include <memory>
#include <vector>
#include <sstream>
#include <atomic>
#include <thread>
#include <bandit/bandit.h>
#include <range/v3/all.hpp>

#include "../source/signals/SignalContext.hpp"

#include "../source/model/Camera.hpp"

using namespace bandit;
using namespace ranges::v3;

using namespace cameramanagercore::signals;
using namespace cameramanagercore::model;

using std::stringstream;
using std::thread;
using std::function;
using std::shared_ptr;
using std::make_shared;
using std::vector;

go_bandit([]{
    describe("Signal context test", [] {
        auto camera = make_shared<Camera>(Camera{"Cam1", "some url",
                                                 "admin", "admin", CameraInteraction::Onvif});

        it("Simple test", [&] {
            stringstream s;
            SignalsContext signal_context;

            signal_context.AddSubscriber<Camera>([&s](SignalData<Camera> signal) { s << signal.data->name; });
            auto generator = signal_context.AddSignalGenerator<Camera>();
            generator->GenerateSignal({ DataStatus::Added, camera });

            AssertThat(s.str(), Is().EqualTo(std::string {"Cam1"}));

        });

        it("Many generators", [&] {
            size_t s = 0;
            SignalsContext signal_context;

            signal_context.AddSubscriber<Camera>([&s](auto signal) { ++s; });

            vector<shared_ptr<SignalGenerator<Camera>>> generators;
            for (auto iteration: view::ints | view::take(100'000))
                generators.emplace_back(signal_context.AddSignalGenerator<Camera>());

            for (auto& generator: generators)
                generator->GenerateSignal({ DataStatus::Added, camera });

            AssertThat(s, Is().EqualTo(100'000));
        });

        it("Many threaded generators", [&] {
            std::atomic_size_t s;
            s.store(0);

            std::atomic_size_t s1;
            s1.store(1'200'000);

            SignalsContext signal_context;
            signal_context.AddSubscriber<Camera>([&s](auto signal) { ++s; });
            signal_context.AddSubscriber<Camera>([&s1](auto signal) { --s1; });

            vector<thread> th;
            for (auto i: view::ints | view::take(12))
                th.emplace_back([&] {
                    vector<shared_ptr<SignalGenerator<Camera>>> generators;
                    for (auto iteration: view::ints | view::take(100'000))
                        generators.emplace_back(signal_context.AddSignalGenerator<Camera>());

                    for (auto& generator: generators)
                        generator->GenerateSignal({ DataStatus::Added, camera });
                });

            for (auto& t: th)
                t.join();

            AssertThat(static_cast<size_t>(s),  Is().EqualTo(1'200'000));
            AssertThat(static_cast<size_t>(s1), Is().EqualTo(0));
        });

        it("Count test", [&] {
            size_t s = 0;
            SignalsContext signal_context;

            for (auto iteration: view::ints | view::take(100'000))
                signal_context.AddSubscriber<Camera>([&s](auto signal) { ++s; });
            AssertThat(signal_context.SubscribersCount<Camera>(), Is().EqualTo(100'000));
            signal_context.RemoveSubscribers<Camera>();
            AssertThat(signal_context.SubscribersCount<Camera>(), Is().EqualTo(0));
            for (auto iteration: view::ints | view::take(100'000))
                signal_context.AddSubscriber<Camera>([&s](auto signal) { ++s; });
            signal_context.RemoveSubscribers();
            AssertThat(signal_context.SubscribersCount<Camera>(), Is().EqualTo(0));

            vector<shared_ptr<SignalGenerator<Camera>>> generators;
            for (auto iteration: view::ints | view::take(100'000))
                generators.emplace_back(signal_context.AddSignalGenerator<Camera>());
            AssertThat(signal_context.SignalGeneratorCount<Camera>(), Is().EqualTo(100'000));
            signal_context.RemoveSignalGenerators<Camera>();
            AssertThat(signal_context.SignalGeneratorCount<Camera>(), Is().EqualTo(0));
            for (auto iteration: view::ints | view::take(100'000))
                generators.emplace_back(signal_context.AddSignalGenerator<Camera>());
            signal_context.RemoveSignalGenerators();
            AssertThat(signal_context.SignalGeneratorCount<Camera>(), Is().EqualTo(0));
        });

        it("Removing test", [&] {
            size_t s = 0;
            SignalsContext signal_context;
            vector<shared_ptr<SignalGenerator<Camera>>> generators;
            for (auto iteration: view::ints | view::take(1'000))
                generators.emplace_back(signal_context.AddSignalGenerator<Camera>());
            AssertThat(signal_context.SignalGeneratorCount<Camera>(), Is().EqualTo(1'000));
            for (auto& generator: generators)
                signal_context.RemoveSignalGenerator<Camera>(generator);
            AssertThat(signal_context.SignalGeneratorCount<Camera>(), Is().EqualTo(0));

            vector<function<void(SignalData<Camera>)>> signals;
            for (auto iteration: view::ints | view::take(1'000))
            {
                signals.emplace_back([](SignalData<Camera> signal) { });
                signal_context.AddSubscriber(signals.back());
            }
            AssertThat(signal_context.SubscribersCount<Camera>(), Is().EqualTo(1'000));
            for (auto& signal: signals)
                signal_context.RemoveSubscriber<Camera>(signal);
            AssertThat(signal_context.SubscribersCount<Camera>(), Is().EqualTo(0));
        });
    });
});