//
// Created by svakhreev on 31.03.17.
//

#ifndef CAMERAMANAGERCORE_CATCHERRORDECORATOR_HPP
#define CAMERAMANAGERCORE_CATCHERRORDECORATOR_HPP

#include <memory>
#include <regex>
#include <exception>
#include <vector>
#include <type_traits>
#include <boost/format.hpp>

#include "../details/ResponseBuilder.hpp"
#include "../../utility/LoggerSettings.hpp"

namespace cameramanagercore::schemas::decorators
{

using namespace cameramanagercore::model;

template < typename ContextsWrapperSpecialized, typename Func >
struct ErrorCatcher
{
    ContextsWrapperSpecialized contexts;
    Func func;

    template < typename Response, typename Request >
    void operator()(std::shared_ptr<Response> response,
                    std::shared_ptr<Request> request)
    {
        static_assert(is_invocable<Func(std::shared_ptr<Response>, std::shared_ptr<Request>)>::value,
                      "Functor must be invokable with (response, request) or (response, request, user)");

        try
        {
            func(response, request);
        }
        catch (std::exception& e)
        {
            LOG(error) << boost::format("Internal server error: %1%") % e.what();
            make_5xx(response, boost::format("Internal server error: %1%") % e.what(), 500);
        }
        catch (...)
        {
            LOG(error) << "Unknown internal server error";
            make_5xx(response, "Unknown internal server error", 500);
        }
    }

};


}

#endif //CAMERAMANAGERCORE_CATCHERRORDECORATOR_HPP
