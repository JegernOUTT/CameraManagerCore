//
// Created by svakhreev on 15.02.17.
//

#include <memory>
#include <list>
#include <tuple>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <thread>
#include <future>
#include <chrono>
#include <sstream>
#include <optional>
#include <nlohmann/json.hpp>

#include <boost/variant.hpp>
#include <boost/format.hpp>
#include <bandit/bandit.h>
#include <range/v3/all.hpp>

#include <onvif_wrapper/Onvif.hpp>
#include "../source/model/Common.hpp"
#include "../source/cameras/CamerasContext.hpp"
#include "../source/cameras/CameraContextVisitors.hpp"
#include "../source/cameras/interactions/Axis.hpp"
#include "../source/thread_pool/pool/ThreadPool.hpp"
#include "../source/thread_pool/ExecutionWrapper.hpp"

using namespace bandit;
using namespace cameramanagercore::model;
using namespace cameramanagercore::cameras;
using namespace cameramanagercore::thread_pool;
using namespace ranges;
using namespace std::chrono;
using namespace nlohmann;

using namespace std::string_literals;
using namespace onvifwrapper;
using boost::variant;
using boost::format;
using boost::str;
using std::get;
using std::make_tuple;
using std::stringstream;
using std::make_shared;
using std::make_unique;
using std::thread;
using std::packaged_task;

using ThreadPoolStd = ThreadPool<>;

go_bandit([]{
    auto signals_context = make_shared<SignalsContext>();

    describe("Camera context test", [&] {
        CamerasContext context { signals_context };
        auto camera1 = make_shared<Camera>(Camera{"Camera 1",
                                                  "http://10.10.20.22//onvif//device_service",
                                                  "admin",
                                                  "admin",
                                                  CameraInteraction::Onvif, 100, "", 1});
        auto camera2 = make_shared<Camera>(Camera{"Camera 2",
                                                  "http://10.10.20.21//onvif//device_service",
                                                  "admin",
                                                  "admin",
                                                  CameraInteraction::Onvif, 100, "", 1});

        before_each([&] {
            context.Clear();
            context.Fill({ camera1, camera2 });
        });

        it("Simple command test", [&] {
            auto [locked, bundle] = context | camera1
                                            | std::make_tuple(GetNodes, ""s);

            AssertThat(static_cast<bool>(bundle), Is().True());
            auto [result, nodes] = bundle.value();

            AssertThat(result.is_error, Is().False());
            AssertThat(nodes.size(), Is().GreaterThan(0));

            LOG(debug) << json(nodes);
        });

        it("ThreadPool test", [&] {
            ThreadPoolStd pool;
            auto maybe_value =
                 pool  | boost::packaged_task<std::optional<tuple<Result, PTZStatus>>>
                         { [&] { auto [locked, bundle] = context | camera1
                                                                 | std::make_tuple(GetStatus, ""s);
                                 return bundle; } }
                       | execute;

            AssertThat(static_cast<bool>(maybe_value), Is().True());
            auto [result, status] = maybe_value.value().value();
            AssertThat(result.is_error, Is().False());
            LOG(debug) << json(status);
        });

        it("ThreadPool test", [&] {
            ThreadPoolOptions options {1, 1024};
            ThreadPoolStd pool(options);
            vector<thread> threads;
            threads.reserve(8);

            auto generator = [&] { auto maybe = pool  | boost::packaged_task<std::optional<tuple<Result, PTZStatus>>>
                                                            { [&] { auto [locked, bundle] = context | camera2
                                                                                                    | std::make_tuple(GetStatus, ""s);
                                                                    return bundle; } }
                                                      | execute;
                                   AssertThat(static_cast<bool>(maybe), Is().True());
                                   auto [status, result] = maybe.value().value();
                                   AssertThat(status.is_error, Is().False());
                                   LOG(debug) << json(result); };

            for (auto stress_iteration: view::ints | view::take(10))
            {
                for (auto repeat: view::ints | view::take(8))
                    threads.emplace_back(thread{ generator });

                for (auto& t: threads)
                    t.join();
                threads.clear();
            }
        });
    });
});