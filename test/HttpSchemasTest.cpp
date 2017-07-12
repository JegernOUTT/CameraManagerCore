//
// Created by svakhreev on 24.03.17.
//

#include <string>
#include <cassert>
#include <unordered_map>
#include <type_traits>
#include <functional>
#include <memory>
#include <future>
#include <chrono>
#include <thread>
#include <vector>
#include <map>
#include <string_view>

#include <bandit/bandit.h>
#include <range/v3/all.hpp>
#include <rttr/registration>
#include <nlohmann/json.hpp>
#include <boost/thread.hpp>

#include "../source/utility/RandomGenerator.hpp"
#include "../source/model/Common.hpp"
#include "../source/schemas/All.hpp"
#include "../source/cameras/CameraContextVisitors.hpp"
#include "../Specializations.hpp"

using namespace bandit;
using namespace ranges;
using namespace rttr;
using namespace std::chrono;
using namespace cameramanagercore::cameras;

using std::move;
using std::make_unique;
using std::string;
using std::vector;
using std::decay_t;
using std::thread;
using std::map;

using namespace cameramanagercore::database::statements;

template<typename T>
struct RandGenerator
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

using nlohmann::json;

go_bandit([] {
    CameraPtrVector               cameras;
    EventPtrVector                events;
    PermissionPtrVector           permissions;
    SettingsPtrVector             settings;
    UserPtrVector                 users;
    UserCamerasPtrVector          user_cameras;
    UserPermissionsPtrVector      user_permissions;
    SessionsPtrVector             sessions;
    CameraActionPtrVector         camera_actions;
    EventArchiveSettingsPtrVector event_settings;
    auto db =      std::make_shared<sqlite::database>("test_data.db");
    auto adapter = std::make_shared<database::adapters::Adapter<sqlite::database>>(db);
    CreateTablesIfNotExist(adapter);
    GetData(adapter, cameras, events, permissions, settings, users, user_cameras,
            user_permissions, sessions, camera_actions, event_settings);
    auto signal_context = std::make_shared<signals::SignalsContext>();
    auto model_context  = std::make_shared<model::Model>(signal_context, cameras, users, settings,
                                                         permissions, user_cameras, user_permissions,
                                                         sessions, camera_actions, event_settings);
    auto event_context = std::make_shared<events::EventContext>(signal_context, events);
    auto pool_context = std::make_shared<thread_pool::ThreadPoolContext<Pool>>();

    auto camera_interaction_pool = std::make_shared<Pool>(thread_pool::ThreadPoolOptions {1, 1024});
    auto schemas_tasks_pool      = std::make_shared<Pool>(thread_pool::ThreadPoolOptions {4, 1024});
    pool_context->pools[thread_pool::ThreadPoolType::Cameras] = camera_interaction_pool;
    pool_context->pools[thread_pool::ThreadPoolType::Common] = schemas_tasks_pool;

    auto cameras_context = std::make_shared<cameras::CamerasContext>(signal_context, cameras);
    auto camera_loop = std::make_shared<CameraLoop>(1s, signal_context, cameras_context, model_context,
                                                    event_context, camera_interaction_pool);
    auto session_context = std::make_shared<SessionContextAsync>(model_context, 1s);
    auto db_subscriber = std::make_shared<DatabaseSubscriber>(signal_context, adapter);
    auto event_subscriber = std::make_shared<EventSubscriber_>(signal_context, event_context,
                                                               cameras_context, camera_interaction_pool,
                                                               model_context);

    auto contexts_wrapper = cameramanagercore::schemas::make_contexts_wrapper(signal_context, model_context, event_context,
                                                                              cameras_context, session_context, pool_context);
    describe("", [&] {
        string action1 = json { Action { ActionType::RelativeMove, {},
                                         {{{1.f, 0.5}, {0.7}}, {{1.f, 1.f}, {1.f}}}, {}, {} } }.dump();
        string action2 = json { Action { ActionType::ContinuousMove, {}, {},
                                         {{{1.f, 0.5}, {0.7}}, 2}, {} } }.dump();
        string action3 = json { Action { ActionType::AbsoluteMove, {}, {}, {},
                                         {{{1.f, 0.5}, {0.7}}, {{0.4, 0.3}, {0.8}}} } }.dump();
        string action4 = json { Action { ActionType::GoToPreset, "1" } }.dump();
        string action5 = json { Action { ActionType::GoToHome } }.dump();
        vector<string> actions = { action1, action2, action3, action4, action5 };

        const auto event_generator = [actions] (int cam)
        {
            using namespace std;
            using namespace std::chrono;

            auto event = RandGenerator<Event>{}();
            event.acknowledged = false;
            event.processed = false;
            event.acknowledged_time = 0l;
            event.processed_time = 0l;

            event.created_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            event.camera_id = cam;
            event.category = "all;";

            vector<string> random_actions = actions | view::sample(2);
            event.on_create_action      = random_actions.front();
            event.on_acknowledge_action = random_actions.back();

            return event;
        };

        it("", [&] {
            using namespace cameramanagercore::schemas;
            using namespace cameramanagercore::schemas::http;
            using namespace cameramanagercore::schemas::ws;
            using namespace cameramanagercore::model;

            using HttpClientServer_      = HttpClientServer< decay_t<decltype(contexts_wrapper)>, LoginSchema, CommonSchema,
                                                             EventsSchema, CamerasSchema >;
            using CameraWebSocketServer_ = CameraWebSocketServer<decay_t<decltype(contexts_wrapper)>>;
            using EventWebSocketServer_ = EventWebSocketServer<decay_t<decltype(contexts_wrapper)>>;
            using ModelWebSocketServer_ = ModelWebSocketServer<decay_t<decltype(contexts_wrapper)>, Camera, User, Permission,
                Settings, UserCameras, UserPermissions, Sessions, CameraAction, EventArchiveSettings>;

            auto settings = model_context->GetSettings();
            auto setting = *settings.front();
            HttpClientServer_ http_server(contexts_wrapper, setting.http_port, 2);
            CameraWebSocketServer_ ws_cameras_server(contexts_wrapper, setting.ws_camera_port, 2);
            EventWebSocketServer_ ws_events_server(contexts_wrapper, setting.ws_events_port, 2);
            ModelWebSocketServer_ ws_model_server(contexts_wrapper, setting.ws_model_port, 2);


            thread http_server_thread([&http_server](){
                http_server.Start();
            });

            thread ws_cameras_thread([&ws_cameras_server](){
                ws_cameras_server.Start();
            });

            thread ws_events_thread([&ws_events_server](){
                ws_events_server.Start();
            });

            thread ws_model_thread([&ws_model_server](){
                ws_model_server.Start();
            });

            thread event_cam1_thread([&](){
                while (true)
                {
                    std::this_thread::sleep_for(RandomGenerator::RandomDuration<seconds>(5, 60));
                    auto created_event = event_context->Create(event_generator(1), "some_session_hash");

                    std::this_thread::sleep_for(RandomGenerator::RandomDuration<seconds>(5, 60));
                    event_context->Change(created_event.value(), EventStatus::Acknowledged, "some_session_hash");
                }
            });

            thread event_cam2_thread([&](){
                while (true)
                {
                    std::this_thread::sleep_for(RandomGenerator::RandomDuration<seconds>(5, 60));
                    auto created_event = event_context->Create(event_generator(2), "some_session_hash");

                    std::this_thread::sleep_for(RandomGenerator::RandomDuration<seconds>(5, 60));
                    event_context->Change(created_event.value(), EventStatus::Acknowledged, "some_session_hash");
                }
            });

            http_server_thread.join();
            ws_cameras_thread.join();
            ws_events_thread.join();
            ws_model_thread.join();
            event_cam1_thread.join();
            event_cam2_thread.join();
        });
    });

});