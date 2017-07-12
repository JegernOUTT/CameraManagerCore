//
// Created by svakhreev on 14.03.17.
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

#include <boost/variant.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <bandit/bandit.h>
#include <range/v3/all.hpp>
#include <nlohmann/json.hpp>

#include <onvif_wrapper/Onvif.hpp>
#include "../source/model/ModelContext.hpp"
#include "../source/cameras/CamerasContext.hpp"
#include "../source/cameras/CameraContextVisitors.hpp"
#include "../source/cameras/interactions/Axis.hpp"
#include "../source/signals/SignalContext.hpp"
#include "../source/cameras/CameraLooper.hpp"
#include "../source/cameras/processors/AllProcessors.hpp"
#include "../source/thread_pool/pool/ThreadPool.hpp"
#include "../source/thread_pool/ExecutionWrapper.hpp"
#include "../source/events/EventContext.hpp"

using namespace bandit;
using namespace nlohmann;
using namespace cameramanagercore::model;
using namespace cameramanagercore::cameras;
using namespace cameramanagercore::thread_pool;
using namespace cameramanagercore::events;
using namespace ranges;
using namespace std::chrono;
using namespace std::string_literals;

using namespace onvifwrapper;
using namespace cameramanagercore::cameras::processors;
using cameramanagercore::model::Camera;
using boost::variant;
using boost::format;
using boost::str;
using std::get;
using std::shared_ptr;
using std::vector;
using std::optional;
using std::stringstream;
using std::make_shared;
using std::make_unique;
using std::thread;
using std::packaged_task;
using std::this_thread::sleep_for;

using ThreadPoolStd = ThreadPool<>;

using CameraStatusProc   = CameraStatusProcessor < SignalsContext, CamerasContext, Model, EventContext, ThreadPoolStd >;
using IdleActionProc     = IdleActionProcessor < SignalsContext, CamerasContext, Model, EventContext, ThreadPoolStd >;
using LockFrameProc      = LockFrameProcessor < SignalsContext, CamerasContext, Model, EventContext, ThreadPoolStd >;
using CameraRecorderProc = CameraRecorderProcessor < SignalsContext, CamerasContext, Model, EventContext, ThreadPoolStd >;
using CameraLoop         = CameraLooper < SignalsContext,
                                          CamerasContext,
                                          Model,
                                          EventContext,
                                          ThreadPoolStd,
                                          CameraStatusProc,
                                          IdleActionProc,
                                          LockFrameProc,
                                          CameraRecorderProc >;

go_bandit([]{
    describe("Camera looper and processors test", [] {
        auto signal_context = make_shared<SignalsContext>();
        auto pool = make_shared<ThreadPoolStd>(ThreadPoolOptions {1, 1024});
        auto cameras_context = make_shared<CamerasContext>(signal_context);
        auto camera1 = make_shared<Camera>(Camera{"Camera 1",
                                                  "http://10.10.20.22//onvif//device_service",
                                                  "admin",
                                                  "admin",
                                                  CameraInteraction::Onvif, 100, "", 1});
        auto camera2 = make_shared<Camera>(Camera{"Camera 2",
                                                  "http://10.10.20.21//onvif//device_service",
                                                  "admin",
                                                  "admin",
                                                  CameraInteraction::Onvif, 100, "", 2});
        auto event_context = make_shared<EventContext>(signal_context, vector<shared_ptr<Event>>{});
        auto model = make_shared<Model>(signal_context, vector<shared_ptr<Camera>> {camera1, camera2});
        auto event_set1 = make_shared<EventArchiveSettings>(EventArchiveSettings { 1, "all;cam1;", 10, 10, 1 });
        auto event_set2 = make_shared<EventArchiveSettings>(EventArchiveSettings { 2, "all;cam2;", 10, 10, 2 });
        auto sets = make_shared<Settings>(Settings { {5, 9, 7, 21, 66, 1, 88, 2}, 50, 15, 16, 78 });
        model->Add(event_set1);
        model->Add(event_set2);
        model->Add(sets);

        shared_ptr<CameraLoop> loop;

        before_each([&] {
            cameras_context->Clear();
            cameras_context->Fill({ camera1, camera2 });
        });

        it("Run / stop looper test", [&] {
            loop = make_shared<CameraLoop>(2s, signal_context, cameras_context, model, event_context, pool);

            for (auto _: view::ints(0, 10))
            {
                loop->Run();
                loop->Stop();
            }

            for (auto _: view::ints(0, 10))
            {
                loop->Run();
            }

            for (auto _: view::ints(0, 10))
            {
                loop->Stop();
            }

            for (auto _: view::ints(0, 10))
            {
                loop->Run();
            }
        });

        it("Run / stop looper test with timeout", [&] {
            loop = make_shared<CameraLoop>(50ms, signal_context, cameras_context, model, event_context, pool);

            for (auto _: view::ints(0, 2))
            {
                loop->Run();
                sleep_for(60s);
                loop->Stop();
            }
        });

        it("CameraStatusProcessor test", [&] {
            using TestLoop = CameraLooper < SignalsContext,
                                            CamerasContext,
                                            Model,
                                            EventContext,
                                            ThreadPoolStd,
                                            CameraStatusProc >;

            auto status_subscriber = [] (SignalData<CameraStatus> data)
            {
                AssertThat(data.status, Is().EqualTo(DataStatus::Added));
                AssertThat(data.data,   Is().Not().EqualTo(nullptr));
                AssertThat(data.data->camera_id, Is().EqualTo(1).Or().EqualTo(2));
            };
            signal_context->AddSubscriber<CameraStatus>(status_subscriber);

            auto _loop = make_shared<TestLoop>(50ms, signal_context, cameras_context, model, event_context, pool);
            sleep_for(10s);

            signal_context->RemoveSubscriber<CameraStatus>(status_subscriber);
        });

        it("IdleActionProcessor test", [&] {
            string action1 = json { Action { ActionType::RelativeMove, {},
                                             {{{1.f, 0.5}, {0.7}}, {{1.f, 1.f}, {1.f}}}, {}, {} } }.dump();
            auto c1 = make_shared<Camera>(Camera{"Camera 1",
                                                 "http://10.10.20.22//onvif//device_service",
                                                 "admin",
                                                 "admin",
                                                 CameraInteraction::Onvif, 5, action1, 1});

            string action2 = json { Action { ActionType::ContinuousMove, {}, {},
                                             {{{1.f, 0.5}, {0.7}}, 2}, {} } }.dump();
            auto c2 = make_shared<Camera>(Camera{"Camera 2",
                                                 "http://10.10.20.21//onvif//device_service",
                                                 "admin",
                                                 "admin",
                                                 CameraInteraction::Onvif, 5, action2, 2});

            string action3 = json { Action { ActionType::AbsoluteMove, {}, {}, {},
                                             {{{1.f, 0.5}, {0.7}}, {{0.4, 0.3}, {0.8}}} } }.dump();
            auto c3 = make_shared<Camera>(Camera{"Camera 3",
                                                 "http://10.10.20.22//onvif//device_service",
                                                 "admin",
                                                 "admin",
                                                 CameraInteraction::Onvif, 5, action3, 3});

            string action4 = json { Action { ActionType::GoToPreset, "1" } }.dump();
            auto c4 = make_shared<Camera>(Camera{"Camera 4",
                                                 "http://10.10.20.21//onvif//device_service",
                                                 "admin",
                                                 "admin",
                                                 CameraInteraction::Onvif, 5, action4, 4});

            string action5 = json { Action { ActionType::GoToHome } }.dump();
            auto c5 = make_shared<Camera>(Camera{"Camera 5",
                                                 "http://10.10.20.22//onvif//device_service",
                                                 "admin",
                                                 "admin",
                                                 CameraInteraction::Onvif, 5, action5, 5});
            auto _model = make_shared<Model>(signal_context, vector<shared_ptr<Camera>> {c1, c2, c3, c4, c5});
            cameras_context->Clear();
            cameras_context->Fill({c1, c2, c3, c4, c5});

            using TestLoop = CameraLooper < SignalsContext,
                                            CamerasContext,
                                            Model,
                                            EventContext,
                                            ThreadPoolStd,
                                            IdleActionProc >;

            auto _loop = make_shared<TestLoop>(50ms, signal_context, cameras_context, _model, event_context, pool);

            auto action_func_1 = [&] () { return (*cameras_context) | c1 | std::make_tuple(GoToHome, ""s, _model); };
            auto action_func_2 = [&] () { return (*cameras_context) | c2 | std::make_tuple(GoToHome, ""s, _model); };
            auto action_func_3 = [&] () { return (*cameras_context) | c3 | std::make_tuple(GoToHome, ""s, _model); };
            auto action_func_4 = [&] () { return (*cameras_context) | c4 | std::make_tuple(GoToHome, ""s, _model); };
            auto action_func_5 = [&] () { return (*cameras_context) | c5 | std::make_tuple(GoToHome, ""s, _model); };

            auto is_true = [&] (SignalData<CameraInIdle> data)
            {
                AssertThat(data.status, Is().EqualTo(DataStatus::Added));
                AssertThat(data.data,   Is().Not().EqualTo(nullptr));
                AssertThat(data.data->camera_id, Is().EqualTo(1).Or().EqualTo(2).Or().EqualTo(3).Or().EqualTo(4).Or().EqualTo(5));
                AssertThat(data.data->in_idle, Is().True());
            };
            auto is_false = [&] (SignalData<CameraInIdle> data)
            {
                AssertThat(data.status, Is().EqualTo(DataStatus::Added));
                AssertThat(data.data,   Is().Not().EqualTo(nullptr));
                AssertThat(data.data->camera_id, Is().EqualTo(1).Or().EqualTo(2).Or().EqualTo(3).Or().EqualTo(4).Or().EqualTo(5));
                AssertThat(data.data->in_idle, Is().False());
            };

            (*pool) | boost::packaged_task<tuple<bool, std::optional<Result>>> { action_func_1 }
                    | execute;
            auto is_false_c1 = [&] (auto&& d) { if (d.data->camera_id == c1->id) is_false(d); };
            auto is_true_c1  = [&] (auto&& d) { if (d.data->camera_id == c1->id) is_true(d); };
            signal_context->AddSubscriber<CameraInIdle>(is_false_c1);
            sleep_for(1s);
            signal_context->RemoveSubscriber<CameraInIdle>(is_false_c1);
            sleep_for(7s);
            signal_context->AddSubscriber<CameraInIdle>(is_true_c1);
            sleep_for(1s);
            signal_context->RemoveSubscriber<CameraInIdle>(is_true_c1);

            (*pool) | boost::packaged_task<tuple<bool, std::optional<Result>>> { action_func_2 }
                    | execute;
            auto is_false_c2 = [&] (auto&& d) { if (d.data->camera_id == c2->id) is_false(d); };
            auto is_true_c2  = [&] (auto&& d) { if (d.data->camera_id == c2->id) is_true(d); };
            signal_context->AddSubscriber<CameraInIdle>(is_false_c2);
            sleep_for(1s);
            signal_context->RemoveSubscriber<CameraInIdle>(is_false_c2);
            sleep_for(7s);
            signal_context->AddSubscriber<CameraInIdle>(is_true_c2);
            sleep_for(1s);
            signal_context->RemoveSubscriber<CameraInIdle>(is_true_c2);

            sleep_for(3s);

            (*pool) | boost::packaged_task<tuple<bool, std::optional<Result>>> { action_func_3 }
                    | execute;
            auto is_false_c3 = [&] (auto&& d) { if (d.data->camera_id == c3->id) is_false(d); };
            auto is_true_c3  = [&] (auto&& d) { if (d.data->camera_id == c3->id) is_true(d); };
            signal_context->AddSubscriber<CameraInIdle>(is_false_c3);
            sleep_for(1s);
            signal_context->RemoveSubscriber<CameraInIdle>(is_false_c3);
            sleep_for(7s);
            signal_context->AddSubscriber<CameraInIdle>(is_true_c3);
            sleep_for(1s);
            signal_context->RemoveSubscriber<CameraInIdle>(is_true_c3);

            sleep_for(3s);

            (*pool) | boost::packaged_task<tuple<bool, std::optional<Result>>> { action_func_4 }
                    | execute;
            auto is_false_c4 = [&] (auto&& d) { if (d.data->camera_id == c4->id) is_false(d); };
            auto is_true_c4  = [&] (auto&& d) { if (d.data->camera_id == c4->id) is_true(d); };
            signal_context->AddSubscriber<CameraInIdle>(is_false_c4);
            sleep_for(1s);
            signal_context->RemoveSubscriber<CameraInIdle>(is_false_c4);
            sleep_for(7s);
            signal_context->AddSubscriber<CameraInIdle>(is_true_c4);
            sleep_for(1s);
            signal_context->RemoveSubscriber<CameraInIdle>(is_true_c4);

            sleep_for(3s);

            (*pool) | boost::packaged_task<tuple<bool, std::optional<Result>>> { action_func_5 }
                    | execute;
            auto is_false_c5 = [&] (auto&& d) { if (d.data->camera_id == c5->id) is_false(d); };
            auto is_true_c5  = [&] (auto&& d) { if (d.data->camera_id == c5->id) is_true(d); };
            signal_context->AddSubscriber<CameraInIdle>(is_false_c5);
            sleep_for(1s);
            signal_context->RemoveSubscriber<CameraInIdle>(is_false_c5);
            sleep_for(7s);
            signal_context->AddSubscriber<CameraInIdle>(is_true_c5);
            sleep_for(1s);
            signal_context->RemoveSubscriber<CameraInIdle>(is_true_c5);
        });

        it("LockFrameProcessor test", [&] {
            using TestLoop = CameraLooper < SignalsContext,
                                            CamerasContext,
                                            Model,
                                            EventContext,
                                            ThreadPoolStd,
                                            LockFrameProc >;

            auto not_locked_cam1 = [&] (SignalData<CameraLockFrame> data)
            {
                if (data.data->camera_id == camera1->id)
                {
                    AssertThat(data.status, Is().EqualTo(DataStatus::Changed));
                    AssertThat(data.data,   Is().Not().EqualTo(nullptr));
                    AssertThat(data.data->locked, Is().False());
                }
            };
            auto locked_cam1 = [&] (SignalData<CameraLockFrame> data)
            {
                if (data.data->camera_id == camera1->id)
                {
                    AssertThat(data.status, Is().EqualTo(DataStatus::Changed));
                    AssertThat(data.data,   Is().Not().EqualTo(nullptr));
                    AssertThat(data.data->locked,  Is().True());
                    AssertThat(data.data->user_id, Is().EqualTo(0x123));
                    AssertThat(data.data->timeout, Is().EqualTo(5));
                }
            };

            auto _loop = make_shared<TestLoop>(10ms, signal_context, cameras_context, model, event_context, pool);
            signal_context->AddSubscriber<CameraLockFrame>(not_locked_cam1);
            sleep_for(4s);
            signal_context->RemoveSubscriber<CameraLockFrame>(not_locked_cam1);

            cameras_context->Lock(camera1, 0x123, 5s);
            signal_context->AddSubscriber<CameraLockFrame>(locked_cam1);
            sleep_for(4s);
            signal_context->RemoveSubscriber<CameraLockFrame>(locked_cam1);
            sleep_for(2s);
            signal_context->AddSubscriber<CameraLockFrame>(not_locked_cam1);
            sleep_for(4s);
            signal_context->RemoveSubscriber<CameraLockFrame>(not_locked_cam1);
        });
    });
});
