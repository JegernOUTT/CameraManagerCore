//
// Created by svakhreev on 20.02.17.
//

#include <memory>
#include <list>
#include <tuple>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <string>
#include <algorithm>

#include <bandit/bandit.h>
#include <rttr/registration>
#include <sqlite_modern_cpp.h>
#include <range/v3/all.hpp>

#include "../source/utility/RandomGenerator.hpp"
#include "../source/model/Camera.hpp"
#include "../source/model/User.hpp"
#include "../source/model/Event.hpp"
#include "../source/model/Settings.hpp"
#include "../source/model/ModelContext.hpp"
#include "../source/database/subscriber/DBSubscriber.hpp"
#include "../source/events/EventSubscriber.hpp"
#include "../source/cameras/CamerasContext.hpp"
#include "../source/events/EventContext.hpp"
#include "../source/thread_pool/ExecutionWrapper.hpp"
#include "../source/thread_pool/pool/ThreadPool.hpp"

#include "../source/database/adapters/SqliteAdapter.hpp"
#include "../source/database/statements/AllStatements.hpp"

using namespace bandit;
using namespace rttr;
using namespace cameramanagercore::database::subscriber;
using namespace cameramanagercore::signals;
using namespace cameramanagercore::database::adapters;
using namespace cameramanagercore::database::statements;
using namespace cameramanagercore::events;
using namespace cameramanagercore::cameras;
using namespace cameramanagercore::model;
using namespace cameramanagercore::thread_pool;

template<typename T>
struct Generator
{
    T operator()()
    {
        T value;
        auto type = type::get<T>();
        for (auto& p: type.get_properties())
        {
            if (p.get_type().is_arithmetic())
            {
                p.set_value(value, generate_num());
            }
            else if (p.get_type() == type::get<string>())
            {
                p.set_value(value, generate_str());
            }
        }
        return value;
    }

    T operator()(const T& value)
    {
        auto type = type::get<T>();
        for (auto& p: type.get_properties())
        {
            if (p.get_type().is_arithmetic())
            {
                p.set_value(value, generate_num());
            }
            else if (p.get_type() == type::get<string>())
            {
                p.set_value(value, generate_str());
            }
        }
        return value;
    }

private:
    int generate_num()
    {
        return RandomGenerator::RandomInt(0, 10'000);
    }

    string generate_str()
    {
        return RandomGenerator::RandomString();
    }
};

using std::make_shared;
using std::shared_ptr;

using AdapterPtr = shared_ptr<Adapter<sqlite::database>>;
using UserPtr = shared_ptr<User>;
using CameraPtr = shared_ptr<Camera>;
using EventPtr = shared_ptr<Event>;
using PermissionPtr = shared_ptr<Permission>;
using SettingsPtr = shared_ptr<Settings>;
using UserCamerasPtr = shared_ptr<UserCameras>;
using UserPermissionsPtr = shared_ptr<UserPermissions>;

using DatabaseSubscriber = DBSubscriber<SignalsContext,
                                        Adapter<sqlite::database>,
                                        Event,
                                        Camera,
                                        User,
                                        Permission,
                                        Settings,
                                        UserCameras,
                                        UserPermissions>;
using EventSubscriber_ = EventSubscriber < SignalsContext,
                                           EventContext,
                                           CamerasContext,
                                           Model,
                                           ThreadPool<> >;

go_bandit([]() {
    describe("Db subscriber tests", [&] {
        auto signals_context = make_shared<SignalsContext>();
        auto db = make_shared<sqlite::database>(":memory:");
        auto adapter = make_shared<Adapter<sqlite::database>>(db);

        auto user1 = make_shared<User>(User{ "Mike",   "12345" });
        auto camera1 = make_shared<Camera>(Camera{ "Mike",   "http://google.com/fgdkbadfnaERASG",
                                                   "admin", "admin", CameraInteraction::Onvif });

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<UserPtr>(adapter);
            CreateTable<CameraPtr>(adapter);
            Add(adapter, { user1 });
            Add(adapter, { camera1 });
        });

        it("Change test", [&] {
            DatabaseSubscriber subscriber(signals_context, adapter);

            camera1->name = "Big camera";
            camera1->url = "I dont know";

            user1->name = "Kolya";
            user1->password = "65a4r6121r5";

            auto camera_generator = signals_context->AddSignalGenerator<Camera>();
            auto user_generator = signals_context->AddSignalGenerator<User>();
            auto event_generator = signals_context->AddSignalGenerator<Event>();
            auto settings_generator = signals_context->AddSignalGenerator<Settings>();

            camera_generator->GenerateSignal(create_signal_data(DataStatus::Changed, camera1));
            user_generator->GenerateSignal(create_signal_data(DataStatus::Changed, user1));

            vector<UserPtr> users;
            SqliteStatus status;
            tie(status, users) = Get<UserPtr>(adapter);
            AssertThat(users.size(), Is().EqualTo(1));
            AssertThat(*users.front(), Is().EqualTo(*user1));

            vector<CameraPtr> cameras;
            tie(status, cameras) = Get<CameraPtr>(adapter);
            AssertThat(cameras.size(), Is().EqualTo(1));
            AssertThat(*cameras.front(), Is().EqualTo(*camera1));
        });

        it("Add test", [&] {
            DatabaseSubscriber subscriber(signals_context, adapter);

            auto user2 = make_shared<User>(User{ "Sergey",   "87494" });
            auto camera2 = make_shared<Camera>(Camera{ "Cam", "http://google.com/asrf125",
                                                       "loh", "loh", CameraInteraction::Onvif });

            auto camera_generator = signals_context->AddSignalGenerator<Camera>();
            auto user_generator = signals_context->AddSignalGenerator<User>();

            camera_generator->GenerateSignal(create_signal_data(DataStatus::Added, camera2));
            user_generator->GenerateSignal(create_signal_data(DataStatus::Added, user2));

            vector<UserPtr> users;
            SqliteStatus status;
            tie(status, users) = Get<UserPtr>(adapter);
            AssertThat(users.size(), Is().EqualTo(2));
            AssertThat(*users[0], Is().EqualTo(*user1));
            AssertThat(*users[1], Is().EqualTo(*user2));

            vector<CameraPtr> cameras;
            tie(status, cameras) = Get<CameraPtr>(adapter);
            AssertThat(cameras.size(), Is().EqualTo(2));
            AssertThat(*cameras[0], Is().EqualTo(*camera1));
            AssertThat(*cameras[1], Is().EqualTo(*camera2));
        });

        it("Remove test", [&] {
            DatabaseSubscriber subscriber(signals_context, adapter);

            auto user2 = make_shared<User>(User{ "Sergey",   "87494" });
            auto camera2 = make_shared<Camera>(Camera{ "Cam", "http://google.com/asrf125",
                                                       "loh", "loh", CameraInteraction::Onvif });

            Add(adapter, { user2 });
            Add(adapter, { camera2 });

            auto camera_generator = signals_context->AddSignalGenerator<Camera>();
            auto user_generator = signals_context->AddSignalGenerator<User>();

            camera_generator->GenerateSignal(create_signal_data(DataStatus::Removed, camera2));
            user_generator->GenerateSignal(create_signal_data(DataStatus::Removed, user2));

            vector<UserPtr> users;
            SqliteStatus status;
            tie(status, users) = Get<UserPtr>(adapter);
            AssertThat(users.size(), Is().EqualTo(1));
            AssertThat(*users[0], Is().EqualTo(*user1));

            vector<CameraPtr> cameras;
            tie(status, cameras) = Get<CameraPtr>(adapter);
            AssertThat(cameras.size(), Is().EqualTo(1));
            AssertThat(*cameras[0], Is().EqualTo(*camera1));
        });

    });

    describe("EventSubscriber tests", [&] {
        shared_ptr<SignalsContext> signals_context;
        shared_ptr<EventContext> event_context;
        shared_ptr<CamerasContext> camera_context;
        shared_ptr<Model> model_context;
        shared_ptr<ThreadPool<>> thread_pool;

        shared_ptr<Event> event1, event2, event3;
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

        string action1 = nlohmann::json { Action { ActionType::RelativeMove, {},
                                         {{{1.f, 0.5}, {0.7}}, {{1.f, 1.f}, {1.f}}}, {}, {} } }.dump();
        string action2 = nlohmann::json { Action { ActionType::ContinuousMove, {}, {},
                                         {{{1.f, 0.5}, {0.7}}, 2}, {} } }.dump();
        string action3 = nlohmann::json { Action { ActionType::AbsoluteMove, {}, {}, {},
                                         {{{1.f, 0.5}, {0.7}}, {{0.4, 0.3}, {0.8}}} } }.dump();
        string action4 = nlohmann::json { Action { ActionType::GoToPreset, "1" } }.dump();
        string action5 = nlohmann::json { Action { ActionType::GoToHome } }.dump();
        vector<string> actions = { action1, action2, action3, action4, action5 };

        before_each([&] {
            signals_context = make_shared<SignalsContext>();
            camera_context = make_shared<CamerasContext>(signals_context, vector<shared_ptr<Camera>>{camera1, camera2});
            thread_pool = make_shared<ThreadPool<>>(ThreadPoolOptions { 1 } );
            model_context = make_shared<Model>(signals_context, vector<shared_ptr<Camera>>{camera1, camera2});
        });

        const auto event_generator = [actions] (int cam)
        {
            using namespace std;
            using namespace std::chrono;

            auto event = Generator<Event>{}();
            event.acknowledged = false;
            event.processed = false;
            event.acknowledged_time = 0l;
            event.processed_time = 0l;

            event.created_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            event.camera_id = cam;

            vector<string> random_actions = actions | view::sample(2);
            event.on_create_action      = random_actions.front();
            event.on_acknowledge_action = random_actions.back();

            return event;
        };

        it("Add test", [&] {
            for (size_t i = 0; i < 2; ++i)
            {
                event_context = make_shared<EventContext>(signals_context, vector<shared_ptr<Event>>{});
                EventSubscriber_ event_subscriber(signals_context, event_context, camera_context, thread_pool, model_context);

                std::thread t1 {[&] {
                    auto event_cam_1 = event_generator(1);
                    vector<Event> events_cam_1 = view::generate_n([&] { std::this_thread::sleep_for(50ms);
                                                                        return event_generator(1); }, 50);
                    events_cam_1 |= action::sort([](auto&& rhs, auto&& lhs) { return rhs.created_time < lhs.created_time; });
                    auto maybe_created_event_cam_1 = event_context->Create(event_cam_1, "some_session_hash");
                    AssertThat(static_cast<bool>(maybe_created_event_cam_1), Is().True());
                    auto created_event_cam_1 = maybe_created_event_cam_1.value();
                    AssertThat(event_context->GetStatus(created_event_cam_1), Is().EqualTo(EventStatus::Processed));
                    for (auto& event: events_cam_1)
                    {
                        auto maybe_created_event = event_context->Create(event, "some_session_hash");
                        AssertThat(static_cast<bool>(maybe_created_event), Is().True());
                        auto created_event = maybe_created_event.value();
                        AssertThat(event_context->GetStatus(created_event), Is().EqualTo(EventStatus::Initial));
                    }

                    auto [ok, ack_event_cam_1] = event_context->Change(created_event_cam_1,
                                                                       EventStatus::Acknowledged,
                                                                       "some_session_hash");
                    AssertThat(ok, Is().True());
                    AssertThat(event_context->GetStatus(ack_event_cam_1), Is().EqualTo(EventStatus::ProcessedAndAcknowledged));
                    auto [ok_false, ack_event_cam_1_] = event_context->Change(ack_event_cam_1, EventStatus::Acknowledged, "some_session_hash");
                    AssertThat(ok_false, Is().False());

                    for (auto& event: events_cam_1)
                    {
                        auto maybe_finded_event = event_context->GetEvent(event.hash);
                        AssertThat(event_context->GetStatus(maybe_finded_event.value()), Is().EqualTo(EventStatus::Processed));
                        auto [ok, acknowledged] = event_context->Change(maybe_finded_event.value(),
                                                                        EventStatus::Acknowledged,
                                                                        "some_session_hash");
                        AssertThat(ok, Is().True());
                        AssertThat(event_context->GetStatus(acknowledged), Is().EqualTo(EventStatus::ProcessedAndAcknowledged));
                        std::this_thread::yield();
                    }

                    for (auto& event: events_cam_1)
                    {
                        bool deleted = event_context->RemoveEvent(event_context->GetEvent(event.hash).value());
                        AssertThat(deleted, Is().True());
                    }
                }};

                std::thread t2 {[&] {
                    auto event_cam_2 = event_generator(2);
                    vector<Event> events_cam_2 = view::generate_n([&] { std::this_thread::sleep_for(50ms);
                                                                        return event_generator(2); }, 10);
                    events_cam_2 |= action::sort([](auto&& rhs, auto&& lhs) { return rhs.created_time < lhs.created_time; });
                    auto maybe_created_event_cam_2 = event_context->Create(event_cam_2, "some_session_hash");
                    AssertThat(static_cast<bool>(maybe_created_event_cam_2), Is().True());
                    auto created_event_cam_2 = maybe_created_event_cam_2.value();
                    AssertThat(event_context->GetStatus(created_event_cam_2), Is().EqualTo(EventStatus::Processed));
                    for (auto& event: events_cam_2)
                    {
                        auto maybe_created_event = event_context->Create(event, "some_session_hash");
                        AssertThat(static_cast<bool>(maybe_created_event), Is().True());
                        auto created_event = maybe_created_event.value();
                        AssertThat(event_context->GetStatus(created_event), Is().EqualTo(EventStatus::Initial));
                    }

                    auto [ok, ack_event_cam_2] = event_context->Change(created_event_cam_2,
                                                                       EventStatus::Acknowledged,
                                                                       "some_session_hash");
                    AssertThat(ok, Is().True());
                    AssertThat(event_context->GetStatus(ack_event_cam_2), Is().EqualTo(EventStatus::ProcessedAndAcknowledged));
                    auto [ok_false, ack_event_cam_2_] = event_context->Change(ack_event_cam_2,
                                                                              EventStatus::Acknowledged,
                                                                              "some_session_hash");
                    AssertThat(ok_false, Is().False());

                    for (auto& event: events_cam_2)
                    {
                        auto maybe_finded_event = event_context->GetEvent(event.hash);
                        AssertThat(event_context->GetStatus(maybe_finded_event.value()), Is().EqualTo(EventStatus::Processed));
                        auto [ok, acknowledged] = event_context->Change(maybe_finded_event.value(),
                                                                        EventStatus::Acknowledged,
                                                                        "some_session_hash");
                        AssertThat(ok, Is().True());
                        AssertThat(event_context->GetStatus(acknowledged), Is().EqualTo(EventStatus::ProcessedAndAcknowledged));
                        std::this_thread::yield();
                    }

                    for (auto& event: events_cam_2)
                    {
                        bool deleted = event_context->RemoveEvent(event_context->GetEvent(event.hash).value());
                        AssertThat(deleted, Is().True());
                    }
                }};

                t1.join();
                t2.join();
            }
        });
    });

});

