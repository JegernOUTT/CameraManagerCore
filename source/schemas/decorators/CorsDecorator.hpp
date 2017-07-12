//
// Created by svakhreev on 04.07.17.
//

#ifndef CAMERAMANAGERCORE_CORSDECORATOR_HPP
#define CAMERAMANAGERCORE_CORSDECORATOR_HPP

#include <memory>
#include <regex>
#include <exception>
#include <vector>
#include <type_traits>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "../details/ResponseBuilder.hpp"
#include "../../utility/LoggerSettings.hpp"

namespace cameramanagercore::schemas::decorators
{

using namespace cameramanagercore::model;

template < typename ContextsWrapperSpecialized, typename Func >
struct CorsChecker
{
    ContextsWrapperSpecialized contexts;
    Func func;

    template < typename Response, typename Request >
    void operator()(std::shared_ptr<Response> response,
                    std::shared_ptr<Request> request)
    {
        static_assert(is_invocable<Func(std::shared_ptr<Response>, std::shared_ptr<Request>)>::value,
                      "Functor must be invokable with (response, request) or (response, request, user)");

        bool is_options = boost::algorithm::to_lower_copy(request->method) == "options";
        bool has_origin = request->header.count("Origin") >= 1;
        bool has_allow_control_methods = request->header.count("Access-Control-Request-Method") >= 1;
        bool has_allow_control_headers = request->header.count("Access-Control-Request-Headers") >= 1;

        std::string origin = "*";
        if (has_origin)
            origin = request->header.find("Origin")->second;

        std::string allowed_headers = "session_hash, content-type";
        if (has_allow_control_headers)
            allowed_headers = request->header.find("Access-Control-Request-Headers")->second;

        bool is_cors_request = is_options
                               && has_origin
                               && has_allow_control_methods
                               && has_allow_control_headers;

        if (is_cors_request)
        {
            make_cors(response, origin, allowed_headers);
        }
        else
            func(response, request);
    }

};


}

#endif //CAMERAMANAGERCORE_CORSDECORATOR_HPP
