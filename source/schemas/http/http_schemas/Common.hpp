//
// Created by svakhreev on 27.03.17.
//

#ifndef CAMERAMANAGERCORE_COMMON_HPP
#define CAMERAMANAGERCORE_COMMON_HPP

#include <memory>
#include <chrono>
#include <vector>

#include <simple-web-server/server_http.hpp>
#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>

#include "../../decorators/SessionCheckerDecorator.hpp"
#include "../../decorators/PermissionCheckerDecorator.hpp"
#include "../../decorators/CatchErrorDecorator.hpp"
#include "../../decorators/CorsDecorator.hpp"
#include "../../details/InputParameters.hpp"
#include "../../details/Utility.hpp"
#include "../../details/ParseFromUrl.hpp"
#include "../../model/OperationResponse.hpp"
#include "../../../session/SessionContext.hpp"
#include "../../../model/User.hpp"
#include "../../../model/Camera.hpp"
#include "../../../model/Permission.hpp"
#include "../../../model/Settings.hpp"
#include "../../../model/User.hpp"

namespace cameramanagercore::schemas::http
{

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using Response = typename HttpServer::Response;
using Request = typename HttpServer::Request;

template < typename C >
struct CommonSchema
{
    template < typename Contexts >
    static void CreateSchemas(Contexts contexts, std::shared_ptr<HttpServer> _server)
    {
        using namespace cameramanagercore::schemas::decorators;

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/common/get_all_cameras/?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req) { GetAllCameras(contexts, res, req); }, contexts);

        set_resource < SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/common/get_my_cameras/?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req, auto u) { GetMyCameras(contexts, res, req, u); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/common/get_all_permissions/?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req) { GetAllPerm(contexts, res, req); }, contexts);

        set_resource < SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/common/get_my_permissions/?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req, auto u) { GetMyPerm(contexts, res, req, u); }, contexts);

        set_resource < SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/common/get_user/?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req, auto u) { GetUser(contexts, res, req, u); }, contexts);

        set_resource < SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/common/get_settings/?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req) { GetSettings(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/common/get_sessions/?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req) { GetSessions(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/common/get_camera_actions/?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req) { GetCameraActions(contexts, res, req); }, contexts);
    }

    template < typename Contexts >
    static void GetAllCameras(Contexts contexts,
                              std::shared_ptr<Response> response,
                              std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to get all cameras schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto all_cameras = model_context->GetCameras();
        std::vector<Camera> copy_cameras = all_cameras | view::transform([] (auto& ptr) { return *ptr; });

        make_200(response, nlohmann::json(copy_cameras));
    }

    template < typename Contexts >
    static void GetMyCameras(Contexts contexts,
                             std::shared_ptr<Response> response,
                             std::shared_ptr<Request> request,
                             std::shared_ptr<cameramanagercore::model::User> user) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;

        LOG(info) << "Request to get my cameras schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto maybe_cameras = model_context->GetCamerasByUser(user);
        if (!maybe_cameras)
        {
            make_200(response, nlohmann::json{});
            return;
        }

        auto cameras = maybe_cameras.value();
        std::vector<Camera> copy_cameras = cameras | view::transform([] (auto& ptr) { return *ptr.lock(); });
        make_200(response, nlohmann::json(copy_cameras));
    }

    template < typename Contexts >
    static void GetAllPerm(Contexts contexts,
                           std::shared_ptr<Response> response,
                           std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to get all permissions schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto all_perms = model_context->GetPermissions();
        std::vector<Permission> copy_perms = all_perms | view::transform([] (auto& ptr) { return *ptr; });

        make_200(response, nlohmann::json(copy_perms));
    }

    template < typename Contexts >
    static void GetMyPerm(Contexts contexts,
                          std::shared_ptr<Response> response,
                          std::shared_ptr<Request> request,
                          std::shared_ptr<cameramanagercore::model::User> user) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;

        LOG(info) << "Request to get my permissions schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto maybe_perms = model_context->GetPermissionsByUser(user);
        if (!maybe_perms)
        {
            make_200(response, nlohmann::json{});
            return;
        }

        auto perms = maybe_perms.value();
        std::vector<Permission> copy_perms = perms | view::transform([] (auto& ptr) { return *ptr.lock(); });
        make_200(response, nlohmann::json(copy_perms));
    }

    template < typename Contexts >
    static void GetUser(Contexts contexts,
                        std::shared_ptr<Response> response,
                        std::shared_ptr<Request> request,
                        std::shared_ptr<cameramanagercore::model::User> user) noexcept
    {
        LOG(info) << "Request to get_user schema";
        make_200(response, nlohmann::json(*user));
    }

    template < typename Contexts >
    static void GetSettings(Contexts contexts,
                            std::shared_ptr<Response> response,
                            std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to get settings schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto all_settings = model_context->GetSettings();
        std::vector<Settings> copy_settings = all_settings | view::transform([] (auto& ptr) { return *ptr; });

        make_200(response, nlohmann::json(copy_settings.front()));
    }

    template < typename Contexts >
    static void GetSessions(Contexts contexts,
                            std::shared_ptr<Response> response,
                            std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to get sessions schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto all_sessions = model_context->GetSessions();
        std::vector<Sessions> copy_sessions = all_sessions | view::transform([] (auto& ptr) { return *ptr; });

        make_200(response, nlohmann::json(copy_sessions));
    }

    template < typename Contexts >
    static void GetCameraActions(Contexts contexts,
                                 std::shared_ptr<Response> response,
                                 std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to get camera actions schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto all_camera_actions = model_context->GetCameraAction();
        std::vector<CameraAction> copy_camera_actions = all_camera_actions | view::transform([] (auto& ptr) { return *ptr; });

        make_200(response, nlohmann::json(copy_camera_actions));
    }
};

}

#endif //CAMERAMANAGERCORE_COMMON_HPP
