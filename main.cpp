//
// Created by svakhreev on 22.03.17.
//

#include <iostream>
#include <exception>
#include <memory>
#include <thread>
#include <vector>
#include <tuple>
#include <chrono>
#include <experimental/optional>
#include <signal.h>
#include <bandit/bandit.h>

#include <boost/stacktrace.hpp>

#include "Specializations.hpp"
#include "source/schemas/All.hpp"

void my_terminate_handler()
{
    LOG(fatal) << "Terminate called:\n"
               << boost::stacktrace::stacktrace()
               << '\n';
    std::cerr << "Terminate called:\n"
              << boost::stacktrace::stacktrace()
              << '\n';
    std::abort();
}

void my_signal_handler(int signum)
{
    ::signal(signum, SIG_DFL);
    boost::stacktrace::stacktrace bt;
    if (bt)
    {
        LOG(fatal) << "Signal "
                   << signum << ", backtrace:\n"
                   << boost::stacktrace::stacktrace() << '\n';

        std::cerr << "Signal "
                  << signum << ", backtrace:\n"
                  << boost::stacktrace::stacktrace() << '\n';
    }
    _Exit(-1);
}

void WaitCycle()
{
    while (true)
    {
        std::this_thread::sleep_for(5s);
    }
}

template <typename ModelContextPtr>
std::experimental::optional<SettingsPtr> GetSettings(ModelContextPtr model)
{
    auto settings_arr = model->GetSettings();
    return settings_arr.size() > 0 ? settings_arr.front() : std::experimental::optional<SettingsPtr>{};
}

int main(int argc, char** argv)
{
    std::set_terminate(&my_terminate_handler);
    ::signal(SIGSEGV,  &my_signal_handler);
    ::signal(SIGABRT,  &my_signal_handler);

    utility::LoggerSettings logger_init(utility::LoggerOutput::FileStdout);

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

    std::string data_base_name = "data.db";
    auto db =      std::make_shared<sqlite::database>(data_base_name);
    auto adapter = std::make_shared<database::adapters::Adapter<sqlite::database>>(db);

    CreateTablesIfNotExist(adapter);
    GetData(adapter, cameras, events, permissions, settings,
            users, user_cameras, user_permissions, sessions,
            camera_actions, event_settings);

    auto signal_context = std::make_shared<signals::SignalsContext>();
    auto model_context  = std::make_shared<model::Model>(signal_context, cameras, users, settings,
                                                         permissions, user_cameras, user_permissions,
                                                         sessions, camera_actions, event_settings);
    auto event_context = std::make_shared<events::EventContext>(signal_context, events);

    auto pool_context = std::make_shared<thread_pool::ThreadPoolContext<Pool>>();
    auto camera_interaction_pool = std::make_shared<Pool>(thread_pool::ThreadPoolOptions {1, 1024});
    pool_context->pools[thread_pool::ThreadPoolType::Cameras] = camera_interaction_pool;

    auto cameras_context = std::make_shared<cameras::CamerasContext>(signal_context, cameras);

    auto camera_loop = std::make_shared<CameraLoop>(1s, signal_context, cameras_context, model_context,
                                                    event_context, camera_interaction_pool);

    auto session_context = std::make_shared<SessionContextAsync>(model_context, 1s);

    auto db_subscriber = std::make_shared<DatabaseSubscriber>(signal_context, adapter);
    auto event_subscriber = std::make_shared<EventSubscriber_>(signal_context, event_context,
                                                               cameras_context, camera_interaction_pool,
                                                               model_context);

    auto contexts_wrapper = cameramanagercore::schemas::make_contexts_wrapper(signal_context, model_context,
                                                                              event_context, cameras_context,
                                                                              session_context, pool_context);

    auto maybe_settings = GetSettings(model_context);
    if (!maybe_settings)
    {
        LOG(fatal) << "Database has no settings, can not continue";
        return -1;
    }

    auto _settings = maybe_settings.value();
    using namespace cameramanagercore::schemas;
    using namespace cameramanagercore::schemas::http;
    using namespace cameramanagercore::schemas::ws;

    using HttpClientServer_ = HttpClientServer < std::decay_t<decltype(contexts_wrapper)>,
                                                 LoginSchema,
                                                 CommonSchema,
                                                 EventsSchema,
                                                 CamerasSchema,
                                                 ModelSchema >;

    using CameraWebSocketServer_ = CameraWebSocketServer < std::decay_t<decltype(contexts_wrapper)>>;
    using EventWebSocketServer_  = EventWebSocketServer < std::decay_t<decltype(contexts_wrapper)>>;
    using ModelWebSocketServer_  = ModelWebSocketServer < std::decay_t<decltype(contexts_wrapper)>,
                                                          cameramanagercore::model::Camera,
                                                          cameramanagercore::model::User,
                                                          cameramanagercore::model::Permission,
                                                          cameramanagercore::model::Settings,
                                                          cameramanagercore::model::UserCameras,
                                                          cameramanagercore::model::UserPermissions,
                                                          cameramanagercore::model::Sessions,
                                                          cameramanagercore::model::CameraAction,
                                                          cameramanagercore::model::EventArchiveSettings >;

    HttpClientServer_ http_server(contexts_wrapper, _settings->http_port, 2);
    CameraWebSocketServer_ ws_cameras_server(contexts_wrapper, _settings->ws_camera_port, 2);
    EventWebSocketServer_ ws_events_server(contexts_wrapper, _settings->ws_events_port, 2);
    ModelWebSocketServer_ ws_model_server(contexts_wrapper, _settings->ws_model_port, 2);

    std::thread http_server_thread([&http_server](){ http_server.Start(); });
    std::thread ws_cameras_thread([&ws_cameras_server](){ ws_cameras_server.Start(); });
    std::thread ws_events_thread([&ws_events_server](){ ws_events_server.Start(); });
    std::thread ws_model_thread([&ws_model_server](){ ws_model_server.Start(); });

    http_server_thread.join();
    ws_cameras_thread.join();
    ws_events_thread.join();
    ws_model_thread.join();

    WaitCycle();

    return 0;
}