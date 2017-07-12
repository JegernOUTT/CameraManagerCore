//
// Created by svakhreev on 28.03.17.
//

#ifndef CAMERAMANAGERCORE_EVENTS_HPP
#define CAMERAMANAGERCORE_EVENTS_HPP

#include <memory>
#include <chrono>
#include <vector>
#include <exception>
#include <simple-web-server/server_http.hpp>
#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>

#include "../../../session/SessionContext.hpp"
#include "../../../model/Event.hpp"
#include "../../../utility/RandomGenerator.hpp"
#include "../../decorators/SessionCheckerDecorator.hpp"
#include "../../decorators/PermissionCheckerDecorator.hpp"
#include "../../decorators/CatchErrorDecorator.hpp"
#include "../../decorators/CorsDecorator.hpp"
#include "../../details/InputParameters.hpp"
#include "../../details/Utility.hpp"
#include "../../details/ParseFromUrl.hpp"
#include "../../model/OperationResponse.hpp"

namespace cameramanagercore::schemas::http
{

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using Response = typename HttpServer::Response;
using Request = typename HttpServer::Request;

template<typename C>
struct EventsSchema
{
    template < typename Contexts >
    static void CreateSchemas(Contexts contexts, std::shared_ptr<HttpServer> _server)
    {
        using namespace cameramanagercore::schemas::decorators;

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/events/create_event\\??(.*)?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req, auto u) { CreateEvent(contexts, res, req, u); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/events/ack_event\\??(.*)?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req, auto u) { AckEvent(contexts, res, req, u); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/events/remove_event\\??(.*)?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req, auto u) { RemoveEvent(contexts, res, req, u); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/events/all/get_events\\??(.*)?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { AllGetEvents(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/events/all/get_periodic_events\\??(.*)?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req) { AllGetEvents(contexts, res, req); }, contexts);

        set_resource < SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/events/my/get_events\\??(.*)?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req, auto u) { MyGetEvents(contexts, res, req, u); }, contexts);

        set_resource < SessionChecker, CorsChecker, ErrorCatcher >
            (_server, "^/events/my/get_periodic_events\\??(.*)?$", {"GET", "OPTIONS"},
             [=] (auto res, auto req, auto u) { MyPeriodicEvents(contexts, res, req, u); }, contexts);
    }


    template < typename Contexts >
    static void CreateEvent(Contexts contexts,
                            std::shared_ptr<Response> response,
                            std::shared_ptr<Request> request,
                            std::shared_ptr<cameramanagercore::model::User> user) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to create event schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto url_parsed = ParseFromUrl(request->path_match[1]);
        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<string, string, int, string, string, string>
                     (url_parsed, "name", "description", "camera_id",
                      "category", "on_create_action", "on_acknowledge_action");
        if (!maybe_input)
        {
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }
        auto [name, description, camera_id, category, on_create_action, on_ack_action] = maybe_input.value();
        Event event { RandomGenerator::RandomString(), name, description, camera_id, category,
                      "", duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(),
                      on_create_action, false, 0, false, "", 0, on_ack_action };
        auto maybe_event = event_context->Create(event, session_hash);
        if (!maybe_event)
        {
            LOG(error) << boost::format("Cannot create event due to some error: %1%") % event;
            make_5xx(response, boost::format("Cannot create event due to some error: %1%") % event, 500);
            return;
        }

        make_200(response, nlohmann::json(maybe_event.value()));
    }

    template < typename Contexts >
    static void AckEvent(Contexts contexts,
                         std::shared_ptr<Response> response,
                         std::shared_ptr<Request> request,
                         std::shared_ptr<cameramanagercore::model::User> user) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to acknowledging event schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto url_parsed = ParseFromUrl(request->path_match[1]);
        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<string>(url_parsed, "event_hash");
        if (!maybe_input)
        {
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [event_hash] = maybe_input.value();
        auto maybe_event = event_context->FindEvent(event_hash);
        if (!maybe_event)
        {
            LOG(error) << boost::format("Cannot find event with requested hash: %1%") % event_hash;
            make_5xx(response, boost::format("Cannot find event with requested hash: %1%") % event_hash, 500);
            return;
        }

        auto finded_event = maybe_event.value();
        auto [success, new_event] = event_context->Change(finded_event, EventStatus::Acknowledged, session_hash);
        if (!success)
        {
            LOG(error) << boost::format("Error while event status changing on acknowledge: %1%") % *finded_event;
            make_5xx(response, boost::format("Error while event status changing on acknowledge: %1%") % *finded_event, 500);
            return;
        }

        make_200(response, nlohmann::json(*new_event));
    }

    template < typename Contexts >
    static void RemoveEvent(Contexts contexts,
                            std::shared_ptr<Response> response,
                            std::shared_ptr<Request> request,
                            std::shared_ptr<cameramanagercore::model::User> user) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to remove event schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto url_parsed = ParseFromUrl(request->path_match[1]);
        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<string>(url_parsed, "event_hash");
        if (!maybe_input)
        {
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }
        auto [event_hash] = maybe_input.value();
        auto maybe_event = event_context->FindEvent(event_hash);
        if (!maybe_event)
        {
            LOG(error) << boost::format("Cannot find event with requested hash: %1%") % event_hash;
            make_5xx(response, boost::format("Cannot find event with requested hash: %1%") % event_hash, 500);
            return;
        }

        auto finded_event = maybe_event.value();
        auto success = event_context->RemoveEvent(finded_event);
        if (!success)
        {
            LOG(error) << boost::format("Error while event removing: %1%") % *finded_event;
            make_5xx(response, boost::format("Error while event removing: %1%") % *finded_event, 500);
            return;
        }

        make_200(response, nlohmann::json(*finded_event));
    }


    template < typename Contexts >
    static void AllGetEvents(Contexts contexts,
                             std::shared_ptr<Response> response,
                             std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to get all events schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto url_parsed = ParseFromUrl(request->path_match[1]);
        auto maybe_input = get_from_url_not_required<int, bool>(url_parsed, "count", "historical");
        if (!maybe_input)
        {
            make_4xx(response, "Error while input parameters parsing", 400);
            return;
        }
        auto [maybe_count, maybe_ishist] = maybe_input.value();
        bool is_hist = maybe_ishist ? maybe_ishist.value() : false;
        int count = maybe_count ? maybe_count.value() : -1;

        std::vector<std::shared_ptr<Event>> events;
        if (is_hist)
        {
            events = event_context->GetArchivedEvents();
        }
        else
        {
            events = event_context->GetEvents();
        }

        if (count != -1)
        {
            events = events | view::take(count);
        }

        make_200(response, nlohmann::json(events));
    }

    template < typename Contexts >
    static void AllPeriodicEvents(Contexts contexts,
                                  std::shared_ptr<Response> response,
                                  std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to get periodic events schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto url_parsed = ParseFromUrl(request->path_match[1]);
        auto [message, maybe_input] = get_from_url<int64_t, int64_t>(url_parsed, "from", "to");
        if (!maybe_input)
        {
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }
        auto [from, to] = maybe_input.value();
        auto events = event_context->GetAllEvents();
        std::vector<std::shared_ptr<Event>> filtered_events = events | view::filter([&, f = from, t = to] (auto& e)
                                                                          { return e->created_time >= f && e->created_time <= t; } );
        make_200(response, nlohmann::json(filtered_events));
    }

    template < typename Contexts >
    static void MyGetEvents(Contexts contexts,
                            std::shared_ptr<Response> response,
                            std::shared_ptr<Request> request,
                            std::shared_ptr<cameramanagercore::model::User> user) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to get all events schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto url_parsed = ParseFromUrl(request->path_match[1]);
        auto maybe_input = get_from_url_not_required<int, bool>(url_parsed, "count", "historical");
        if (!maybe_input)
        {
            make_4xx(response, "Error while input parameters parsing", 400);
            return;
        }
        auto [maybe_count, maybe_ishist] = maybe_input.value();
        bool is_hist = maybe_ishist ? maybe_ishist.value() : false;
        int count = maybe_count ? maybe_count.value() : -1;

        std::vector<std::shared_ptr<Event>> events;
        if (is_hist)
        {
            events = event_context->GetArchivedEvents();
        }
        else
        {
            events = event_context->GetEvents();
        }

        if (count != -1)
        {
            events = events | view::take(count);
        }

        std::vector<string> allowed_categories = user->event_categories | view::split(';');
        auto filtered_events = events | view::filter([&] (auto& e)
                                                     {
                                                         std::vector<string> all;
                                                         std::vector<string> e_categories = e->category | view::split(';');
                                                         v3::set_intersection(allowed_categories, e_categories, v3::back_inserter(all));
                                                         return all.size() > 0;
                                                     });
        make_200(response, nlohmann::json(filtered_events));
    }

    template < typename Contexts >
    static void MyPeriodicEvents(Contexts contexts,
                                 std::shared_ptr<Response> response,
                                 std::shared_ptr<Request> request,
                                 std::shared_ptr<cameramanagercore::model::User> user) noexcept
    {
        using namespace cameramanagercore::model;
        using namespace cameramanagercore::session;
        using namespace std::chrono;
        using namespace ranges;

        LOG(info) << "Request to get periodic events schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;

        auto url_parsed = ParseFromUrl(request->path_match[1]);
        auto [message, maybe_input] = get_from_url<int64_t, int64_t>(url_parsed, "from", "to");
        if (!maybe_input)
        {
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }
        auto [from, to] = maybe_input.value();

        auto events = event_context->GetAllEvents();
        std::vector<std::shared_ptr<Event>> events_ = events | view::filter([&, f = from, t = to] (auto& e)
                                                                  { return e->created_time >= f && e->created_time <= t; } );

        std::vector<string> allowed_categories = user->event_categories | view::split(';');
        auto filtered_events = events_ | view::filter([&] (auto& e)
                                                      {
                                                          std::vector<string> all;
                                                          std::vector<string> e_categories = e->category | view::split(';');
                                                          v3::set_intersection(allowed_categories, e_categories, v3::back_inserter(all));
                                                          return all.size() > 0;
                                                      });
        make_200(response, nlohmann::json(filtered_events));
    }
};

}

#endif //CAMERAMANAGERCORE_EVENTS_HPP
