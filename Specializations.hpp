//
// Created by svakhreev on 10.01.17.
//

#ifndef CAMERAMANAGERCORE_APPLICATIONGRAPH_HPP
#define CAMERAMANAGERCORE_APPLICATIONGRAPH_HPP

#include <iostream>
#include <exception>
#include <memory>
#include <vector>
#include <tuple>
#include <chrono>
#include <signal.h>
#include <sqlite_modern_cpp.h>
#include <onvif_wrapper/Onvif.hpp>

#include "source/cameras/parcers/JsonOnvifParcer.hpp"
#include "source/model/parcers/JsonParcer.hpp"

#include "source/utility/LoggerSettings.hpp"

#include "source/database/adapters/SqliteAdapter.hpp"
#include "source/database/statements/AllStatements.hpp"
#include "source/database/utility/SqliteUtility.hpp"

#include "source/model/Action.hpp"
#include "source/model/Camera.hpp"
#include "source/model/CameraInformation.hpp"
#include "source/model/CameraLockFrame.hpp"
#include "source/model/Event.hpp"
#include "source/model/Permission.hpp"
#include "source/model/Settings.hpp"
#include "source/model/User.hpp"
#include "source/model/UserPermissions.hpp"
#include "source/model/UserCameras.hpp"
#include "source/model/ModelContext.hpp"
#include "source/model/Sessions.hpp"
#include "source/model/EventArchiveSettings.hpp"

#include "source/signals/SignalContext.hpp"

#include "source/events/EventContext.hpp"

#include "source/database/subscriber/DBSubscriber.hpp"
#include "source/events/EventSubscriber.hpp"

#include "source/thread_pool/pool/ThreadPool.hpp"
#include "source/thread_pool/ExecutionWrapper.hpp"
#include "source/thread_pool/ThreadPoolContext.hpp"

#include "source/cameras/CamerasContext.hpp"
#include "source/cameras/CameraContextVisitors.hpp"
#include "source/cameras/interactions/Axis.hpp"

#include "source/cameras/CameraLooper.hpp"
#include "source/cameras/processors/AllProcessors.hpp"

#include "source/session/Session.hpp"
#include "source/session/SessionContext.hpp"

using namespace cameramanagercore;
using namespace std::chrono_literals;

using UserPtr                        = std::shared_ptr<model::User>;
using CameraPtr                      = std::shared_ptr<model::Camera>;
using EventPtr                       = std::shared_ptr<model::Event>;
using PermissionPtr                  = std::shared_ptr<model::Permission>;
using SettingsPtr                    = std::shared_ptr<model::Settings>;
using UserCamerasPtr                 = std::shared_ptr<model::UserCameras>;
using UserPermissionsPtr             = std::shared_ptr<model::UserPermissions>;
using CameraActionPtr                = std::shared_ptr<model::CameraAction>;
using CameraStatusPtr                = std::shared_ptr<model::CameraStatus>;
using CameraInIdlePtr                = std::shared_ptr<model::CameraInIdle>;
using CameraLockFramePtr             = std::shared_ptr<model::CameraLockFrame>;
using SessionsPtr                    = std::shared_ptr<model::Sessions>;
using EventArchiveSettingsPtr        = std::shared_ptr<model::EventArchiveSettings>;

using UserWPtr                       = std::weak_ptr<model::User>;
using CameraWPtr                     = std::weak_ptr<model::Camera>;
using EventWPtr                      = std::weak_ptr<model::Event>;
using PermissionWPtr                 = std::weak_ptr<model::Permission>;
using SettingsWPtr                   = std::weak_ptr<model::Settings>;
using UserCamerasWPtr                = std::weak_ptr<model::UserCameras>;
using UserPermissionsWPtr            = std::weak_ptr<model::UserPermissions>;
using CameraActionWPtr               = std::weak_ptr<model::CameraAction>;
using CameraStatusWPtr               = std::weak_ptr<model::CameraStatus>;
using CameraInIdleWPtr               = std::weak_ptr<model::CameraInIdle>;
using CameraLockFrameWPtr            = std::weak_ptr<model::CameraLockFrame>;
using SessionsWPtr                   = std::weak_ptr<model::Sessions>;
using EventArchiveSettingsWPtr       = std::weak_ptr<model::EventArchiveSettings>;

using UserPtrVector                  = std::vector<std::shared_ptr<model::User>>;
using CameraPtrVector                = std::vector<std::shared_ptr<model::Camera>>;
using EventPtrVector                 = std::vector<std::shared_ptr<model::Event>>;
using PermissionPtrVector            = std::vector<std::shared_ptr<model::Permission>>;
using SettingsPtrVector              = std::vector<std::shared_ptr<model::Settings>>;
using UserCamerasPtrVector           = std::vector<std::shared_ptr<model::UserCameras>>;
using UserPermissionsPtrVector       = std::vector<std::shared_ptr<model::UserPermissions>>;
using CameraActionPtrVector          = std::vector<std::shared_ptr<model::CameraAction>>;
using CameraStatusPtrVector          = std::vector<std::shared_ptr<model::CameraStatus>>;
using CameraInIdlePtrVector          = std::vector<std::shared_ptr<model::CameraInIdle>>;
using CameraLockFramePtrVector       = std::vector<std::shared_ptr<model::CameraLockFrame>>;
using SessionsPtrVector              = std::vector<std::shared_ptr<model::Sessions>>;
using EventArchiveSettingsPtrVector  = std::vector<std::shared_ptr<model::EventArchiveSettings>>;

using SqliteAdapter = database::adapters::Adapter<sqlite::database>;

using ModelsTypesTuple = std::tuple< model::Camera,
                                     model::Event,
                                     model::Permission,
                                     model::Settings,
                                     model::User,
                                     model::UserCameras,
                                     model::UserPermissions,
                                     model::Sessions,
                                     model::CameraAction,
                                     model::EventArchiveSettings >;

using ModelsTypesPtrTuple = std::tuple< std::shared_ptr<model::Camera>,
                                        std::shared_ptr<model::Event>,
                                        std::shared_ptr<model::Permission>,
                                        std::shared_ptr<model::Settings>,
                                        std::shared_ptr<model::User>,
                                        std::shared_ptr<model::UserCameras>,
                                        std::shared_ptr<model::UserPermissions>,
                                        std::shared_ptr<model::Sessions>,
                                        std::shared_ptr<model::CameraAction>,
                                        std::shared_ptr<model::EventArchiveSettings> >;

using DatabaseSubscriber = database::subscriber::DBSubscriber < signals::SignalsContext,
                                                                SqliteAdapter,
                                                                model::Event,
                                                                model::Camera,
                                                                model::User,
                                                                model::Permission,
                                                                model::Settings,
                                                                model::UserCameras,
                                                                model::UserPermissions,
                                                                model::Sessions,
                                                                model::CameraAction,
                                                                model::EventArchiveSettings >;

using Pool = thread_pool::ThreadPool<>;

using EventSubscriber_ = events::EventSubscriber < signals::SignalsContext,
                                                   events::EventContext,
                                                   cameras::CamerasContext,
                                                   model::Model,
                                                   Pool >;

using CameraStatusProc = cameras::processors::CameraStatusProcessor < signals::SignalsContext,
                                                                      cameras::CamerasContext,
                                                                      model::Model,
                                                                      events::EventContext,
                                                                      Pool >;
using IdleActionProc   = cameras::processors::IdleActionProcessor   < signals::SignalsContext,
                                                                      cameras::CamerasContext,
                                                                      model::Model,
                                                                      events::EventContext,
                                                                      Pool >;
using LockFrameProc    = cameras::processors::LockFrameProcessor    < signals::SignalsContext,
                                                                      cameras::CamerasContext,
                                                                      model::Model,
                                                                      events::EventContext,
                                                                      Pool >;
using CameraRecorderProc    = cameras::processors::CameraRecorderProcessor    < signals::SignalsContext,
                                                                                cameras::CamerasContext,
                                                                                model::Model,
                                                                                events::EventContext,
                                                                                Pool >;
using CameraLoop       = cameras::CameraLooper < signals::SignalsContext,
                                                 cameras::CamerasContext,
                                                 model::Model,
                                                 events::EventContext,
                                                 Pool,
                                                 CameraStatusProc,
                                                 IdleActionProc,
                                                 LockFrameProc,
                                                 CameraRecorderProc >;

using Generator           = session::SecreteKeyGenerator<session::random_alg>;
using SessionSpec         = session::Session<Generator, std::chrono::seconds, UserPtr>;
using SessionSpecPtr      = std::shared_ptr<SessionSpec>;
using SessionContextLazy  = session::SessionContext < SessionSpecPtr,
                                                      std::chrono::seconds,
                                                      session::SessionClearPolicy::Lazy,
                                                      model::Model >;
using SessionContextAsync = session::SessionContext < SessionSpecPtr,
                                                      std::chrono::seconds,
                                                      session::SessionClearPolicy::Async,
                                                      model::Model >;

template<typename Adapter>
void CreateTablesIfNotExist(std::shared_ptr<Adapter> adapter)
{
    database::statements::CreateTable<UserPtr>(adapter)
        | database::statements::OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
        | database::statements::Check   {};

    database::statements::CreateTable<CameraPtr>(adapter)
        | database::statements::OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
        | database::statements::Check   {};

    database::statements::CreateTable<EventPtr>(adapter)
        | database::statements::OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
        | database::statements::Check   {};

    database::statements::CreateTable<PermissionPtr>(adapter)
        | database::statements::OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
        | database::statements::Check   {};

    database::statements::CreateTable<SettingsPtr>(adapter)
        | database::statements::OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
        | database::statements::Check   {};

    database::statements::CreateTable<UserCamerasPtr>(adapter)
        | database::statements::OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
        | database::statements::Check   {};

    database::statements::CreateTable<UserPermissionsPtr>(adapter)
        | database::statements::OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
        | database::statements::Check   {};

    database::statements::CreateTable<SessionsPtr>(adapter)
        | database::statements::OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
        | database::statements::Check   {};

    database::statements::CreateTable<CameraActionPtr>(adapter)
        | database::statements::OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
        | database::statements::Check   {};

    database::statements::CreateTable<EventArchiveSettingsPtr>(adapter)
        | database::statements::OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
        | database::statements::Check   {};
}

template<typename Adapter>
void GetData(std::shared_ptr<Adapter>        adapter,
             CameraPtrVector&                cameras,
             EventPtrVector&                 events,
             PermissionPtrVector&            permissions,
             SettingsPtrVector&              settings,
             UserPtrVector&                  users,
             UserCamerasPtrVector&           user_cameras,
             UserPermissionsPtrVector&       user_permissions,
             SessionsPtrVector&              sessions,
             CameraActionPtrVector&          camera_actions,
             EventArchiveSettingsPtrVector&  event_settings)
{
    auto [s1, _cameras] = database::statements::Get<CameraPtr>(adapter);
    s1 | database::statements::OnSuccess { [&, _cameras = _cameras] (auto& s) { cameras = _cameras; } }
       | database::statements::OnError   { [&]                      (auto& s) { cameras = {}; } }
       | database::statements::Check     {};

    auto [s2, _events] = database::statements::Get<EventPtr>(adapter);
    s2 | database::statements::OnSuccess { [&, _events = _events] (auto& s) { events = _events; } }
       | database::statements::OnError   { [&]                    (auto& s) { events = {}; } }
       | database::statements::Check     {};

    auto [s3, _permissions] = database::statements::Get<PermissionPtr>(adapter);
    s3 | database::statements::OnSuccess { [&, _permissions = _permissions] (auto& s) { permissions = _permissions; } }
       | database::statements::OnError   { [&]                              (auto& s) { permissions = {}; } }
       | database::statements::Check     {};

    auto [s4, _settings] = database::statements::Get<SettingsPtr>(adapter);
    s4 | database::statements::OnSuccess { [&, _settings = _settings] (auto& s) { settings = _settings; } }
       | database::statements::OnError   { [&]                        (auto& s) { settings = {}; } }
       | database::statements::Check     {};

    auto [s5, _users] = database::statements::Get<UserPtr>(adapter);
    s5 | database::statements::OnSuccess { [&, _users = _users] (auto& s) { users = _users; } }
       | database::statements::OnError   { [&]                  (auto& s) { users = {}; } }
       | database::statements::Check     {};

    auto [s6, _user_cameras] = database::statements::Get<UserCamerasPtr>(adapter);
    s6 | database::statements::OnSuccess { [&, _user_cameras = _user_cameras] (auto& s) { user_cameras = _user_cameras; } }
       | database::statements::OnError   { [&]                                (auto& s) { user_cameras = {}; } }
       | database::statements::Check     {};

    auto [s7, _user_permissions] = database::statements::Get<UserPermissionsPtr>(adapter);
    s7 | database::statements::OnSuccess { [&, _user_permissions = _user_permissions] (auto& s) { user_permissions = _user_permissions; } }
       | database::statements::OnError   { [&]                                        (auto& s) { user_permissions = {}; } }
       | database::statements::Check     {};

    auto [s8, _sessions] = database::statements::Get<SessionsPtr>(adapter);
    s8 | database::statements::OnSuccess { [&, _sessions = _sessions] (auto& s) { sessions = _sessions; } }
       | database::statements::OnError   { [&]                        (auto& s) { sessions = {}; } }
       | database::statements::Check     {};

    auto [s9, _camera_actions] = database::statements::Get<CameraActionPtr>(adapter);
    s9 | database::statements::OnSuccess { [&, _camera_actions = _camera_actions] (auto& s) { camera_actions = _camera_actions; } }
       | database::statements::OnError   { [&]                                    (auto& s) { camera_actions = {}; } }
       | database::statements::Check     {};

    auto [s10, _event_settings] = database::statements::Get<EventArchiveSettingsPtr>(adapter);
    s10 | database::statements::OnSuccess { [&, _event_settings = _event_settings] (auto& s) { event_settings = _event_settings; } }
        | database::statements::OnError   { [&]                                    (auto& s) { event_settings = {}; } }
        | database::statements::Check     {};
}

#endif //CAMERAMANAGERCORE_APPLICATIONGRAPH_HPP
