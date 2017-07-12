//
// Created by svakhreev on 28.03.17.
//

#ifndef CAMERAMANAGERCORE_MODEL_HPP
#define CAMERAMANAGERCORE_MODEL_HPP


#include <memory>
#include <chrono>

#include <simple-web-server/server_http.hpp>
#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>

#include "../../decorators/SessionCheckerDecorator.hpp"
#include "../../decorators/CatchErrorDecorator.hpp"
#include "../../decorators/CorsDecorator.hpp"
#include "../../details/InputParameters.hpp"
#include "../../details/ParseFromUrl.hpp"
#include "../../details/Utility.hpp"
#include "../../model/OperationResponse.hpp"
#include "../../../session/SessionContext.hpp"

#include "../../../model/Camera.hpp"
#include "../../../model/User.hpp"
#include "../../../model/Settings.hpp"
#include "../../../model/Permission.hpp"
#include "../../../model/UserCameras.hpp"
#include "../../../model/UserPermissions.hpp"
#include "../../../model/Sessions.hpp"
#include "../../../model/CameraInformation.hpp"
#include "../../../model/EventArchiveSettings.hpp"

namespace cameramanagercore::schemas::http
{

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using Response = typename HttpServer::Response;
using Request = typename HttpServer::Request;

template < typename C >
struct ModelSchema
{
    template < typename Contexts >
    static void CreateSchemas(Contexts contexts, std::shared_ptr<HttpServer> _server)
    {
        using namespace cameramanagercore::schemas::decorators;
        using namespace cameramanagercore::model;

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/camera/get/?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { Get<Camera>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/user/get/?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { Get<User>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/settings/get/?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { Get<Settings>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/permission/get/?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { Get<Permission>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/user_cameras/get/?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { Get<UserCameras>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/user_permissions/get/?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { Get<UserPermissions>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/sessions/get/?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { Get<Sessions>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/camera_action/get/?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { Get<CameraAction>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/event_archive/get/?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { Get<EventArchiveSettings>(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/camera/add\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Add<Camera>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/user/add\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Add<User>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/settings/add\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Add<Settings>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/permission/add\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Add<Permission>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/user_cameras/add\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Add<UserCameras>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/user_permissions/add\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Add<UserPermissions>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/sessions/add\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Add<Sessions>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/camera_action/add\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Add<CameraAction>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/event_archive/add\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Add<EventArchiveSettings>(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/camera/change\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Change<Camera>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/user/change\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Change<User>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/settings/change\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Change<Settings>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/permission/change\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Change<Permission>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/sessions/change\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Change<Sessions>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/camera_action/change\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Change<CameraAction>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/event_archive/change\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Change<EventArchiveSettings>(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/camera/remove\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Remove<Camera>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/user/remove\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Remove<User>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/settings/remove\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Remove<Settings>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/permission/remove\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Remove<Permission>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/user_cameras/remove\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Remove<UserCameras>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/user_permissions/remove\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Remove<UserPermissions>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/sessions/remove\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Remove<Sessions>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/camera_action/remove\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Remove<CameraAction>(contexts, res, req); }, contexts);
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/model/event_archive/remove\\??(.*)?$", {"POST", "OPTIONS"},
             [=] (auto res, auto req) { Remove<EventArchiveSettings>(contexts, res, req); }, contexts);
    }

    template < typename ModelType, typename Contexts >
    static void Get(Contexts contexts,
                    std::shared_ptr<Response> response,
                    std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to get data from model";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        std::vector<std::shared_ptr<ModelType>> data;
        if constexpr (std::is_same_v<ModelType, Camera>)
        {
            data = model_context->GetCameras();
            LOG(error) << data.size();
        }
        if constexpr (std::is_same_v<ModelType, User>)
        {
            data = model_context->GetUsers();
        }
        if constexpr (std::is_same_v<ModelType, Settings>)
        {
            data = model_context->GetSettings();
        }
        if constexpr (std::is_same_v<ModelType, Permission>)
        {
            data = model_context->GetPermissions();
        }
        if constexpr (std::is_same_v<ModelType, UserCameras>)
        {
            data = model_context->GetUserCameras();
        }
        if constexpr (std::is_same_v<ModelType, UserPermissions>)
        {
            data = model_context->GetUserPermissions();
        }
        if constexpr (std::is_same_v<ModelType, Sessions>)
        {
            data = model_context->GetSessions();
        }
        if constexpr (std::is_same_v<ModelType, CameraAction>)
        {
            data = model_context->GetCameraAction();
        }
        if constexpr (std::is_same_v<ModelType, EventArchiveSettings>)
        {
            data = model_context->GetEventArchiveSettings();
        }

        std::vector<ModelType> data_ = data | view::transform([] (auto&& ptr) { return *ptr; });
        make_200(response, nlohmann::json(data_));
    }

    template < typename ModelType, typename Contexts >
    static void Add(Contexts contexts,
                    std::shared_ptr<Response> response,
                    std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;

        LOG(info) << "Request to add data to model";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto url_parsed = ParseFromUrl(request->path_match[1]);

        if constexpr (std::is_same_v<ModelType, Camera>)
        {
            auto [message, maybe_input] = get_from_url<std::string, std::string, std::string, std::string, int, int, std::string>(
                url_parsed, "name", "url", "login", "password", "interaction_id", "idle_timeout_sec", "on_idle_action");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [name, url, login, password, interaction_id, idle_timeout_sec, on_idle_action] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(
                ModelType { name, url, login, password, static_cast<CameraInteraction>(interaction_id), idle_timeout_sec, on_idle_action });
            auto status = model_context->Add(obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not add object to model: %1%") % *obj;
                make_5xx(response, boost::format("Can not add object to model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*obj));
        }
        if constexpr (std::is_same_v<ModelType, User>)
        {
            auto [message, maybe_input] = get_from_url<std::string, std::string, std::string>(
                url_parsed, "name", "password", "event_categories");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [name, password, event_categories] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { name, password, event_categories });
            auto status = model_context->Add(obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not add object to model: %1%") % *obj;
                make_5xx(response, boost::format("Can not add object to model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*obj));
        }
        if constexpr (std::is_same_v<ModelType, Settings>)
        {
            auto [message, maybe_input] = get_from_url<int, int, int, int>(
                url_parsed, "http_port", "ws_camera_port", "ws_model_port", "ws_events_port");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [http_port, ws_camera_port, ws_model_port, ws_events_port] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { {}, http_port, ws_camera_port,
                                                               ws_model_port, ws_events_port });
            auto status = model_context->Add(obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not add object to model: %1%") % *obj;
                make_5xx(response, boost::format("Can not add object to model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*obj));
        }
        if constexpr (std::is_same_v<ModelType, Permission>)
        {
            auto [message, maybe_input] = get_from_url<std::string, std::string>(
                url_parsed, "name", "regex_url");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [name, regex_url] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { name, regex_url });
            auto status = model_context->Add(obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not add object to model: %1%") % *obj;
                make_5xx(response, boost::format("Can not add object to model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*obj));
        }
        if constexpr (std::is_same_v<ModelType, UserCameras>)
        {
            auto [message, maybe_input] = get_from_url<int32_t, int32_t>(
                url_parsed, "user_id", "camera_id");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [user_id, camera_id] = maybe_input.value();
            auto maybe_camera = model_context->FindCameraById(camera_id);
            auto maybe_user = model_context->FindUserById(user_id);

            if (!maybe_camera || !maybe_user)
            {
                make_4xx(response, boost::format(
                    "Can not find user or camera by id: camera_id: %1%, user_id: %2%") % camera_id % user_id, 404);
                return;
            }
            auto status = model_context->AddLink(maybe_user.value(), maybe_camera.value());
            auto obj = std::make_shared<ModelType>(ModelType { user_id, camera_id });

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not add object to model: %1%") % *obj;
                make_5xx(response, boost::format("Can not add object to model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*obj));
        }
        if constexpr (std::is_same_v<ModelType, UserPermissions>)
        {
            auto [message, maybe_input] = get_from_url<int32_t, int32_t>(
                url_parsed, "user_id", "permission_id");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [user_id, permission_id] = maybe_input.value();
            auto maybe_perm = model_context->FindPermissionById(permission_id);
            auto maybe_user = model_context->FindUserById(user_id);

            if (!maybe_perm || !maybe_user)
            {
                make_4xx(response, boost::format(
                    "Can not find user or permission by id: permission: %1%, user_id: %2%") % permission_id % user_id, 404);
                return;
            }
            auto status = model_context->AddLink(maybe_user.value(), maybe_perm.value());
            auto obj = std::make_shared<ModelType>(ModelType { user_id, permission_id });

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not add object to model: %1%") % *obj;
                make_5xx(response, boost::format("Can not add object to model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*obj));
        }
        if constexpr (std::is_same_v<ModelType, Sessions>)
        {
            auto [message, maybe_input] = get_from_url<std::string, int, SqliteDateTime, SqliteDateTime>(
                url_parsed, "session_hash", "user_id", "creation_time", "destruction_time");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [session_hash, user_id, creation_time, destruction_time] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { session_hash, user_id, creation_time, destruction_time });
            auto status = model_context->Add(obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not add object to model: %1%") % *obj;
                make_5xx(response, boost::format("Can not add object to model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*obj));
        }
        if constexpr (std::is_same_v<ModelType, CameraAction>)
        {
            auto [message, maybe_input] = get_from_url<bool, int, std::string, SqliteDateTime, std::string>(
                url_parsed, "succeed", "camera_id", "session_hash", "time", "action");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [succeed, camera_id, session_hash, time, action] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { succeed, camera_id, session_hash, time, action });
            auto status = model_context->Add(obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not add object to model: %1%") % *obj;
                make_5xx(response, boost::format("Can not add object to model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*obj));
        }
        if constexpr (std::is_same_v<ModelType, EventArchiveSettings>)
        {
            auto [message, maybe_input] = get_from_url<int, std::string, int, int>(
                url_parsed, "camera_id", "event_categories", "buffer_seconds", "record_seconds");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [camera_id, event_categories, buffer_seconds, record_seconds] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { camera_id, event_categories, buffer_seconds, record_seconds });
            auto status = model_context->Add(obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not add object to model: %1%") % *obj;
                make_5xx(response, boost::format("Can not add object to model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*obj));
        }
    }

    template < typename ModelType, typename Contexts >
    static void Change(Contexts contexts,
                       std::shared_ptr<Response> response,
                       std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;

        LOG(info) << "Request to change data from model";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto url_parsed = ParseFromUrl(request->path_match[1]);

        if constexpr (std::is_same_v<ModelType, Camera>)
        {
            auto [message, maybe_input] = get_from_url<int, std::string, std::string, std::string, std::string, int, int, std::string>(
                url_parsed, "id", "name", "url", "login", "password", "interaction_id", "idle_timeout_sec", "on_idle_action");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id, name, url, login, password, interaction_id, idle_timeout_sec, on_idle_action] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(
                ModelType { name, url, login, password, static_cast<CameraInteraction>(interaction_id), idle_timeout_sec, on_idle_action });
            auto maybe_old = model_context->FindCameraById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto [status, changed] = model_context->Change(maybe_old.value(), obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not change object at model: %1%") % *obj;
                make_5xx(response, boost::format("Can not change object at model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*changed));
        }
        if constexpr (std::is_same_v<ModelType, User>)
        {
            auto [message, maybe_input] = get_from_url<int, std::string, std::string, std::string>(
                url_parsed, "id", "name", "password", "event_categories");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id, name, password, event_categories] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { name, password, event_categories });
            auto maybe_old = model_context->FindUserById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto [status, changed] = model_context->Change(maybe_old.value(), obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not change object at model: %1%") % *obj;
                make_5xx(response, boost::format("Can not change object at model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*changed));
        }
        if constexpr (std::is_same_v<ModelType, Settings>)
        {
            auto [message, maybe_input] = get_from_url<int, int, int, int, int>(
                url_parsed, "id", "http_port", "ws_camera_port", "ws_model_port", "ws_events_port");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id, http_port, ws_camera_port, ws_model_port, ws_events_port] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { {}, http_port, ws_camera_port,
                                                               ws_model_port, ws_events_port });
            auto maybe_old = model_context->FindSettingsById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto [status, changed] = model_context->Change(maybe_old.value(), obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not change object at model: %1%") % *obj;
                make_5xx(response, boost::format("Can not change object at model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*changed));
        }
        if constexpr (std::is_same_v<ModelType, Permission>)
        {
            auto [message, maybe_input] = get_from_url<int, std::string, std::string>(
                url_parsed, "id", "name", "regex_url");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id, name, regex_url] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { name, regex_url });
            auto maybe_old = model_context->FindPermissionById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto [status, changed] = model_context->Change(maybe_old.value(), obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not change object at model: %1%") % *obj;
                make_5xx(response, boost::format("Can not change object at model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*changed));
        }
        if constexpr (std::is_same_v<ModelType, Sessions>)
        {
            auto [message, maybe_input] = get_from_url<std::string, std::string, int, SqliteDateTime, SqliteDateTime>(
                url_parsed, "old_session_hash", "session_hash", "user_id", "creation_time", "destruction_time");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [old_session_hash, session_hash, user_id, creation_time, destruction_time] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { session_hash, user_id, creation_time, destruction_time });
            auto maybe_old = model_context->FindSessionByHash(old_session_hash);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % old_session_hash, 404);
                return;
            }
            auto [status, changed] = model_context->Change(maybe_old.value(), obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not change object at model: %1%") % *obj;
                make_5xx(response, boost::format("Can not change object at model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*changed));
        }
        if constexpr (std::is_same_v<ModelType, CameraAction>)
        {
            auto [message, maybe_input] = get_from_url<int, bool, int, std::string, SqliteDateTime, std::string>(
                url_parsed, "id", "succeed", "camera_id", "session_hash", "time", "action");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id, succeed, camera_id, session_hash, time, action] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { succeed, camera_id, session_hash, time, action });
            auto maybe_old = model_context->FindCameraActionById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto [status, changed] = model_context->Change(maybe_old.value(), obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not change object at model: %1%") % *obj;
                make_5xx(response, boost::format("Can not change object at model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*changed));
        }
        if constexpr (std::is_same_v<ModelType, EventArchiveSettings>)
        {
            auto [message, maybe_input] = get_from_url<int, int, std::string, int, int>(
                url_parsed, "id", "camera_id", "event_categories", "buffer_seconds", "record_seconds");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id, camera_id, event_categories, buffer_seconds, record_seconds] = maybe_input.value();
            auto obj = std::make_shared<ModelType>(ModelType { camera_id, event_categories, buffer_seconds, record_seconds });
            auto maybe_old = model_context->FindEventArchiveSettingsById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto [status, changed] = model_context->Change(maybe_old.value(), obj);

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not change object at model: %1%") % *obj;
                make_5xx(response, boost::format("Can not change object at model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*changed));
        }
    }

    template < typename ModelType, typename Contexts >
    static void Remove(Contexts contexts,
                       std::shared_ptr<Response> response,
                       std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;

        LOG(info) << "Request to remove data from model";

        auto [signal_context, model_context, event_context,
            cameras_context, session_context, pool_context] = contexts.contexts;

        auto url_parsed = ParseFromUrl(request->path_match[1]);


        if constexpr (std::is_same_v<ModelType, Camera>)
        {
            auto [message, maybe_input] = get_from_url<int>(url_parsed, "id");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id] = maybe_input.value();
            auto maybe_old = model_context->FindCameraById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto status = model_context->Remove(maybe_old.value());

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not remove object from model: %1%") % *maybe_old;
                make_5xx(response, boost::format("Can not remove object from model: %1%") % *maybe_old, 500);
                return;
            }
            make_200(response, nlohmann::json(*maybe_old));
        }
        if constexpr (std::is_same_v<ModelType, User>)
        {
            auto [message, maybe_input] = get_from_url<int>(url_parsed, "id");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id] = maybe_input.value();
            auto maybe_old = model_context->FindUserById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto status = model_context->Remove(maybe_old.value());

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not remove object from model: %1%") % *maybe_old;
                make_5xx(response, boost::format("Can not remove object from model: %1%") % *maybe_old, 500);
                return;
            }
            make_200(response, nlohmann::json(*maybe_old));
        }
        if constexpr (std::is_same_v<ModelType, Settings>)
        {
            auto [message, maybe_input] = get_from_url<int>(url_parsed, "id");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id] = maybe_input.value();
            auto maybe_old = model_context->FindSettingsById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto status = model_context->Remove(maybe_old.value());

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not remove object from model: %1%") % *maybe_old;
                make_5xx(response, boost::format("Can not remove object from model: %1%") % *maybe_old, 500);
                return;
            }
            make_200(response, nlohmann::json(*maybe_old));
        }
        if constexpr (std::is_same_v<ModelType, Permission>)
        {
            auto [message, maybe_input] = get_from_url<int>(url_parsed, "id");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id] = maybe_input.value();
            auto maybe_old = model_context->FindPermissionById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto status = model_context->Remove(maybe_old.value());

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not remove object from model: %1%") % *maybe_old;
                make_5xx(response, boost::format("Can not remove object from model: %1%") % *maybe_old, 500);
                return;
            }
            make_200(response, nlohmann::json(*maybe_old));
        }
        if constexpr (std::is_same_v<ModelType, UserCameras>)
        {
            auto [message, maybe_input] = get_from_url<int32_t, int32_t>(
                url_parsed, "user_id", "camera_id");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [user_id, camera_id] = maybe_input.value();
            auto maybe_camera = model_context->FindCameraById(camera_id);
            auto maybe_user = model_context->FindUserById(user_id);

            if (!maybe_camera || !maybe_user)
            {
                make_4xx(response, boost::format(
                    "Can not find user or camera by id: camera_id: %1%, user_id: %2%") % camera_id % user_id, 404);
                return;
            }
            auto status = model_context->RemoveLink(maybe_user.value(), maybe_camera.value());
            auto obj = std::make_shared<ModelType>(ModelType {user_id, camera_id});

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not remove object from model: %1%") % *obj;
                make_5xx(response, boost::format("Can not remove object from model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*obj));
        }
        if constexpr (std::is_same_v<ModelType, UserPermissions>)
        {
            auto [message, maybe_input] = get_from_url<int32_t, int32_t>(
                url_parsed, "user_id", "permission_id");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [user_id, permission_id] = maybe_input.value();
            auto maybe_perm = model_context->FindPermissionById(permission_id);
            auto maybe_user = model_context->FindUserById(user_id);

            if (!maybe_perm || !maybe_user)
            {
                make_4xx(response, boost::format(
                    "Can not find user or permission by id: permission: %1%, user_id: %2%") % permission_id % user_id, 404);
                return;
            }
            auto status = model_context->RemoveLink(maybe_user.value(), maybe_perm.value());
            auto obj = std::make_shared<ModelType>(ModelType {user_id, permission_id});

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not add object from model: %1%") % *obj;
                make_5xx(response, boost::format("Can not add object from model: %1%") % *obj, 500);
                return;
            }
            make_200(response, nlohmann::json(*obj));
        }
        if constexpr (std::is_same_v<ModelType, Sessions>)
        {
            auto [message, maybe_input] = get_from_url<std::string>(url_parsed, "session_hash");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [hash] = maybe_input.value();
            auto maybe_old = model_context->FindSessionByHash(hash);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested hash: %1%") % hash, 404);
                return;
            }
            auto status = model_context->Remove(maybe_old.value());

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not remove object from model: %1%") % *maybe_old;
                make_5xx(response, boost::format("Can not remove object from model: %1%") % *maybe_old, 500);
                return;
            }
            make_200(response, nlohmann::json(*maybe_old));
        }
        if constexpr (std::is_same_v<ModelType, CameraAction>)
        {
            auto [message, maybe_input] = get_from_url<int>(url_parsed, "int");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id] = maybe_input.value();
            auto maybe_old = model_context->FindCameraActionById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto status = model_context->Remove(maybe_old.value());

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not remove object from model: %1%") % *maybe_old;
                make_5xx(response, boost::format("Can not remove object from model: %1%") % *maybe_old, 500);
                return;
            }
            make_200(response, nlohmann::json(*maybe_old));
        }
        if constexpr (std::is_same_v<ModelType, EventArchiveSettings>)
        {
            auto [message, maybe_input] = get_from_url<int>(url_parsed, "id");
            if (!maybe_input)
            {
                make_4xx(response, boost::format(
                    "Can not find valid input parameters for this schema: %1%") % message, 400);
                return;
            }
            auto [id] = maybe_input.value();
            auto maybe_old = model_context->FindEventArchiveSettingsById(id);
            if (!maybe_old)
            {
                make_4xx(response, boost::format("Can not find object with requested id: %1%") % id, 404);
                return;
            }
            auto status = model_context->Remove(maybe_old.value());

            if (status != OperationStatus::Ok)
            {
                LOG(error) << boost::format("Can not remove object from model: %1%") % *maybe_old;
                make_5xx(response, boost::format("Can not remove object from model: %1%") % *maybe_old, 500);
                return;
            }
            make_200(response, nlohmann::json(*maybe_old));
        }
    }
};

}

#endif //CAMERAMANAGERCORE_MODEL_HPP
