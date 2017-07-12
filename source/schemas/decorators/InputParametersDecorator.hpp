////
//// Created by svakhreev on 31.03.17.
////
//
//#ifndef CAMERAMANAGERCORE_INPUTPARAMETERSDECORATOR_HPP
//#define CAMERAMANAGERCORE_INPUTPARAMETERSDECORATOR_HPP
//
//#include <memory>
//#include <exception>
//#include <type_traits>
//#include <experimental/string_view>
//#include <nlohmann/json.hpp>
//#include <boost/format.hpp>
//
//#include "../details/ResponseBuilder.hpp"
//#include "../details/InputParameters.hpp"
//#include "../../utility/LoggerSettings.hpp"
//#include "../../model/User.hpp"
//#include "../../model/ModelContext.hpp"
//#include "../../session/SessionContext.hpp"
//#include "../../utility/FuncUtility.hpp"
//
//namespace cameramanagercore::schemas::decorators
//{
//
//using std::shared_ptr;
//using std::exception;
//using std::decay_t;
//using std::forward;
//using std::experimental::string_view;
//using boost::format;
//using namespace nlohmann;
//using namespace cameramanagercore::session;
//using namespace cameramanagercore::model;
//using namespace cameramanagercore::utility;
//
//using Generator           = session::SecreteKeyGenerator<session::random_alg>;
//using SessionSpec         = session::Session<Generator, std::chrono::seconds, shared_ptr<User>>;
//using SessionSpecPtr      = std::shared_ptr<SessionSpec>;
//using SessionContextLazy  = session::SessionContext< SessionSpecPtr, std::chrono::seconds,
//    session::SessionClearPolicy::Lazy, Model >;
//using SessionContextAsync = session::SessionContext< SessionSpecPtr, std::chrono::seconds,
//    session::SessionClearPolicy::Async, Model >;
//
//template <typename... Types>
//struct InputParameters
//{
//    array<string> parameter_names;
//
//    using
//
//};
//
//template < typename ContextsWrapperSpecialized,
//           typename Func,
//           typename... Types >
//struct InputParameters
//{
//    ContextsWrapperSpecialized contexts;
//    Func func;
//
//    template <typename Response, typename Request>
//    void operator()(shared_ptr<Response> response, shared_ptr<Request> request)
//    {
//        static_assert(is_invocable_v<Func(shared_ptr<Response>, shared_ptr<Request>)>
//                      || is_invocable_v<Func(shared_ptr<Response>, shared_ptr<Request>, shared_ptr<User>)> ,
//                      "Functor must be invokable with (response, request) or (response, request, user)");
//
//        auto session_context = contexts.template Get<SessionContextAsync>();
//        auto [message, maybe_session_hash] = get_from_header<string>(request, "session_hash");
//
//        if (!maybe_session_hash)
//        {
//            make_400(response, format("Can not find valid input parameters for this schema: %1%") % message);
//            return;
//        }
//        auto [session_hash] = maybe_session_hash.value();
//        auto maybe_session = session_context->FindSession(session_hash);
//
//        if (!maybe_session)
//        {
//            LOG(error) << format("Error while session checking: can not "
//                                     "find sessions with requested hash. Request: %1%") % message;
//            make_400(response, format("Session not found with requested session hash %1%") % session_hash);
//            return;
//        }
//        auto session = maybe_session.value();
//        session->Touch();
//        LOG(info) << format("Finded session with requested hash: %1%. User: %2%")
//                     % session_hash % *(session->GetUser());
//
//        if constexpr (is_invocable_v<Func(shared_ptr<Response>, shared_ptr<Request>)>)
//        {
//            func(response, request);
//        }
//        else if (is_invocable_v<Func(shared_ptr<Response>, shared_ptr<Request>, shared_ptr<User>)>)
//        {
//            func(response, request, session->GetUser());
//        }
//    }
//
//};
//
//}
//
//#endif //CAMERAMANAGERCORE_INPUTPARAMETERSDECORATOR_HPP
