//
// Created by svakhreev on 24.03.17.
//

#ifndef CAMERAMANAGERCORE_SESSIONCHECKER_HPP
#define CAMERAMANAGERCORE_SESSIONCHECKER_HPP

#include <memory>
#include <exception>
#include <type_traits>
#include <string_view>
#include <nlohmann/json.hpp>
#include <boost/format.hpp>

#include "../details/ResponseBuilder.hpp"
#include "../details/InputParameters.hpp"
#include "../../utility/LoggerSettings.hpp"
#include "../../model/User.hpp"
#include "../../model/ModelContext.hpp"
#include "../../session/SessionContext.hpp"
#include "../../utility/FuncUtility.hpp"

namespace cameramanagercore::schemas::decorators
{

using namespace cameramanagercore::session;
using namespace cameramanagercore::model;
using namespace cameramanagercore::utility;

using Generator           = session::SecreteKeyGenerator<session::random_alg>;
using SessionSpec         = session::Session<Generator, std::chrono::seconds, std::shared_ptr<User>>;
using SessionSpecPtr      = std::shared_ptr<SessionSpec>;
using SessionContextLazy  = session::SessionContext< SessionSpecPtr, std::chrono::seconds,
                                                     session::SessionClearPolicy::Lazy, Model >;
using SessionContextAsync = session::SessionContext< SessionSpecPtr, std::chrono::seconds,
                                                     session::SessionClearPolicy::Async, Model >;

template < typename ContextsWrapperSpecialized, typename Func >
struct SessionChecker
{
    ContextsWrapperSpecialized contexts;
    Func func;

    template <typename Response, typename Request>
    void operator()(std::shared_ptr<Response> response, std::shared_ptr<Request> request)
    {
        static_assert(is_invocable_v<Func(std::shared_ptr<Response>, std::shared_ptr<Request>)>
                      || is_invocable_v<Func(std::shared_ptr<Response>, std::shared_ptr<Request>, std::shared_ptr<User>)> ,
                      "Functor must be invokable with (response, request) or (response, request, user)");

        auto session_context = contexts.template Get<SessionContextAsync>();
        auto [message, maybe_session_hash] = get_from_header<string>(request, "session_hash");

        if (!maybe_session_hash)
        {
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }
        auto [session_hash] = maybe_session_hash.value();
        auto maybe_session = session_context->FindSession(session_hash);

        if (!maybe_session)
        {
            LOG(error) << boost::format("Error while session checking: can not "
                                            "find sessions with requested hash. Request: %1%") % message;
            make_4xx(response, boost::format("Session not found with this session hash: %1%") % session_hash, 401);
            return;
        }
        auto session = maybe_session.value();
        session->Touch();
        LOG(info) << boost::format("Founded session with hash: %1%. User: %2%")
                     % session_hash % *(session->GetUser());

        if constexpr (is_invocable_v<Func(std::shared_ptr<Response>, std::shared_ptr<Request>)>)
        {
            func(response, request);
        }
        else if (is_invocable_v<Func(std::shared_ptr<Response>, std::shared_ptr<Request>, std::shared_ptr<User>)>)
        {
            func(response, request, session->GetUser());
        }
    }

};

}

#endif //CAMERAMANAGERCORE_SESSIONCHECKER_HPP
