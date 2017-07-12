//
// Created by svakhreev on 28.03.17.
//

#ifndef CAMERAMANAGERCORE_PERMISSIONCHECKERDECORATOR_HPP
#define CAMERAMANAGERCORE_PERMISSIONCHECKERDECORATOR_HPP

#include <memory>
#include <regex>
#include <exception>
#include <vector>
#include <type_traits>
#include <string_view>

#include <nlohmann/json.hpp>
#include <boost/format.hpp>
#include <range/v3/all.hpp>

#include "../details/ResponseBuilder.hpp"
#include "../details/InputParameters.hpp"
#include "../../utility/LoggerSettings.hpp"
#include "../../model/User.hpp"
#include "../../model/Permission.hpp"
#include "../../model/ModelContext.hpp"
#include "../../session/SessionContext.hpp"
#include "../../utility/FuncUtility.hpp"

namespace cameramanagercore::schemas::decorators
{

using namespace cameramanagercore::model;

template <typename ContextsWrapperSpecialized, typename Func>
struct PermissionChecker
{
    ContextsWrapperSpecialized contexts;
    Func func;

    template <typename Response, typename Request>
    void operator()(std::shared_ptr<Response> response,
                    std::shared_ptr<Request> request,
                    std::shared_ptr<User> user)
    {
        using namespace ranges;

        static_assert(is_invocable_v<Func(std::shared_ptr<Response>, std::shared_ptr<Request>)>
                      || is_invocable_v<Func(std::shared_ptr<Response>, std::shared_ptr<Request>, std::shared_ptr<User>)> ,
                      "Functor must be invokable with (response, request) or (response, request, user)");

        auto model_context = contexts.template Get<Model>();
        auto maybe_permissions = model_context->GetPermissionsByUser(user);
        if (!maybe_permissions)
        {
            LOG(error) << boost::format("User has no any permissions: %1%") % *user;
            make_4xx(response, boost::format("User has no any permissions: %1%") % *user, 403);
            return;
        }
        auto perms = maybe_permissions.value();
        std::vector<Permission> perms_no_ptr = perms | view::transform([] (auto& ptr) { return *ptr.lock(); });
        auto permited = v3::any_of(perms_no_ptr,
                                   [&] (auto& permission)
                                   {
                                       return std::regex_search(request->path, std::regex { permission.regex_url });
                                   });
        if (!permited)
        {
            LOG(error) << boost::format("User %1% has no  permission to schema: %2%") % *user % request->path;
            make_4xx(response, boost::format("User %1% has no  permission to schema: %2%") % *user % request->path, 403);
            return;
        }

        if constexpr (is_invocable_v<Func(std::shared_ptr<Response>, std::shared_ptr<Request>)>)
        {
            func(response, request);
        }
        else if (is_invocable_v<Func(std::shared_ptr<Response>, std::shared_ptr<Request>, std::shared_ptr<User>)>)
        {
            func(response, request, user);
        }
    }

};


}

#endif //CAMERAMANAGERCORE_PERMISSIONCHECKERDECORATOR_HPP
