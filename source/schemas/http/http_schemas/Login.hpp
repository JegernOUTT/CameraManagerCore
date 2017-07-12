//
// Created by svakhreev on 24.03.17.
//

#ifndef CAMERAMANAGERCORE_LOGIN_HPP
#define CAMERAMANAGERCORE_LOGIN_HPP

#include <memory>
#include <chrono>

#include <simple-web-server/server_http.hpp>
#include <nlohmann/json.hpp>

#include "../../decorators/SessionCheckerDecorator.hpp"
#include "../../decorators/CatchErrorDecorator.hpp"
#include "../../decorators/CorsDecorator.hpp"
#include "../../details/InputParameters.hpp"
#include "../../details/ParseFromUrl.hpp"
#include "../../details/Utility.hpp"
#include "../../model/OperationResponse.hpp"
#include "../../../session/SessionContext.hpp"
#include "../../../model/User.hpp"

namespace cameramanagercore::schemas::http
{

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using Response = typename HttpServer::Response;
using Request = typename HttpServer::Request;

template<typename C>
struct LoginSchema
{
    template < typename Contexts >
    static void CreateSchemas(Contexts contexts, std::shared_ptr<HttpServer> _server)
    {
        using namespace cameramanagercore::schemas::decorators;

        set_resource < ErrorCatcher, CorsChecker >
            (_server, "^/login\\??(.*)?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { Login(contexts, res, req); }, contexts);

        set_resource < SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/logout/?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req, auto u) { Logout(contexts, res, req, u); }, contexts);
    }

    template < typename Contexts >
    static void Login(Contexts contexts,
                      std::shared_ptr<Response> response,
                      std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;

        LOG(info) << "Request to login schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto url_parsed = ParseFromUrl(request->path_match[1]);
        auto [message, maybe_input] = get_from_url<std::string, std::string>(url_parsed, "name", "password");

        if (!maybe_input)
        {
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }
        auto [login, password] = maybe_input.value();

        auto maybe_finded = model_context->FindUserByNamePassword({ login, password });
        if (!maybe_finded)
        {
            LOG(error) << boost::format("User with requested name / password not found: %1%") % message;
            make_5xx(response, boost::format("User with requested name / password not found: %1%") % message, 500);
            return;
        }

        auto session = make_session(1000s, maybe_finded.value());
        session_context->AddSession(session);

        nlohmann::json answer;
        answer["session_hash"] = session->Secrete();

        make_200(response, answer);
    }

    template < typename Contexts >
    static void Logout(Contexts contexts,
                       std::shared_ptr<Response> response,
                       std::shared_ptr<Request> request,
                       std::shared_ptr<cameramanagercore::model::User> user) noexcept
    {
        LOG(info) << "Request to logout schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto [message, maybe_session_hash] = get_from_header<std::string>(request, "session_hash");
        auto [session_hash] = maybe_session_hash.value_or(
            std::remove_reference_t<decltype(maybe_session_hash)>::value_type {});

        session_context->RemoveSession(session_hash);

        make_200(response, "Session successfully removed");
    }
};

}

#endif //CAMERAMANAGERCORE_LOGIN_HPP
