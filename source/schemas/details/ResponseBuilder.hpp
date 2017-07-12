//
// Created by svakhreev on 24.03.17.
//

#ifndef CAMERAMANAGERCORE_RESPONSEBUILDER_HPP
#define CAMERAMANAGERCORE_RESPONSEBUILDER_HPP

#include <memory>
#include <type_traits>
#include <string_view>

#include <boost/format.hpp>
#include <nlohmann/json.hpp>

#include "../model/OperationResponse.hpp"
#include "../../utility/LoggerSettings.hpp"

namespace cameramanagercore::schemas
{

using namespace model;

template <typename ResponseStream, typename MessageType>
struct Make200;

template <typename ResponseStream>
struct Make200 <ResponseStream, nlohmann::json>
{
    std::shared_ptr<ResponseStream> response;
    nlohmann::json answer;

    void operator()()
    {
        auto r = answer.dump(2);
        *response << "HTTP/1.1 200 OK\r\n"
                  << "Content-Type: application/json\r\n"
                  << "Access-Control-Allow-Origin: *\r\n"
                  << "Access-Control-Allow-Headers: *\r\n"
                  << "Access-Control-Allow-Methods: *\r\n"
                  << "Content-Length: " << r.length() << "\r\n\r\n"
                  << r;
    }
};

template <typename ResponseStream>
struct Make200 <ResponseStream, const char *>
{
    std::shared_ptr<ResponseStream> response;
    std::string_view answer;

    void operator()()
    {
        nlohmann::json r = OperationResponse { 200, answer.data() };
        auto dumped = r.dump(2);

        *response << "HTTP/1.1 200 OK\r\n"
                  << "Content-Type: application/json\r\n"
                  << "Access-Control-Allow-Origin: *\r\n"
                  << "Access-Control-Allow-Headers: *\r\n"
                  << "Access-Control-Allow-Methods: *\r\n"
                  << "Content-Length: " << dumped.length() << "\r\n\r\n"
                  << dumped;
    }
};


template <typename ResponseStream, typename MessageType>
struct Make4xx;

template <typename ResponseStream>
struct Make4xx <ResponseStream, const char *>
{
    std::shared_ptr<ResponseStream> response;
    std::string_view error_message;

    void operator()(int code)
    {
        nlohmann::json r = OperationResponse { code, error_message.data() };
        auto dumped = r.dump(2);

        std::string code_repr = "";
        switch (code)
        {
            case 400: code_repr = "400 Bad Request"; break;  // On any unrecognized schema
            case 401: code_repr = "401 Unauthorized"; break; // On invalid session hash
            case 403: code_repr = "403 Forbidden"; break;    // On permission issues
            case 404: code_repr = "404 Not Found"; break;    // On required parameters not found
            default: code_repr = "400 Bad Request"; break;
        }
        *response << (boost::format("HTTP/1.1 %1%\r\n") % code_repr).str()
                  << "Content-Type: application/json\r\n"
                  << "Access-Control-Allow-Origin: *\r\n"
                  << "Access-Control-Allow-Headers: *\r\n"
                  << "Access-Control-Allow-Methods: *\r\n"
                  << "Content-Length: " << dumped.length() << "\r\n\r\n"
                  << dumped;
    }
};

template <typename ResponseStream>
struct Make4xx <ResponseStream, boost::format>
{
    std::shared_ptr<ResponseStream> response;
    boost::format error_message;

    void operator()(int code)
    {
        nlohmann::json r = OperationResponse { code, error_message.str() };
        auto dumped = r.dump(2);

        std::string code_repr = "";
        switch (code)
        {
            case 400: code_repr = "400 Bad Request"; break;  // On required parameters not found
            case 401: code_repr = "401 Unauthorized"; break; // On invalid session hash
            case 403: code_repr = "403 Forbidden"; break;    // On permission issues
            case 404: code_repr = "404 Not Found"; break;    // On any unrecognized schema
            default: code_repr = "400 Bad Request"; break;
        }
        *response << (boost::format("HTTP/1.1 %1%\r\n") % code_repr).str()
                  << "Content-Type: application/json\r\n"
                  << "Access-Control-Allow-Origin: *\r\n"
                  << "Access-Control-Allow-Headers: *\r\n"
                  << "Access-Control-Allow-Methods: *\r\n"
                  << "Content-Length: " << dumped.length() << "\r\n\r\n"
                  << dumped;
    }
};


template <typename ResponseStream, typename MessageType>
struct Make5xx;

template <typename ResponseStream>
struct Make5xx <ResponseStream, const char *>
{
    std::shared_ptr<ResponseStream> response;
    std::string_view error_message;

    void operator()(int code)
    {
        nlohmann::json r = OperationResponse { code, error_message.data() };
        auto dumped = r.dump(2);

        std::string code_repr = "";
        switch (code)
        {
            case 500: code_repr = "500 Internal Server Error"; break; // On any unrecognized error
            case 501: code_repr = "501 Not Implemented"; break;       // On not implemented method
            case 503: code_repr = "503 Service Unavailable"; break;   // On some integrations unavailable
            default: code_repr = "500 Internal Server Error"; break;
        }
        *response << (boost::format("HTTP/1.1 %1%\r\n") % code_repr).str()
                  << "Content-Type: application/json\r\n"
                  << "Access-Control-Allow-Origin: *\r\n"
                  << "Access-Control-Allow-Headers: *\r\n"
                  << "Access-Control-Allow-Methods: *\r\n"
                  << "Content-Length: " << dumped.length() << "\r\n\r\n"
                  << dumped;
    }
};

template <typename ResponseStream>
struct Make5xx <ResponseStream, boost::format>
{
    std::shared_ptr<ResponseStream> response;
    boost::format error_message;

    void operator()(int code)
    {
        nlohmann::json r = OperationResponse { code, error_message.str() };
        auto dumped = r.dump(2);

        std::string code_repr = "";
        switch (code)
        {
            case 500: code_repr = "500 Internal Server Error"; break; // On any unrecognized error
            case 501: code_repr = "501 Not Implemented"; break;       // On not implemented method
            case 503: code_repr = "503 Service Unavailable"; break;   // On some integrations unavailable
            default: code_repr = "500 Internal Server Error"; break;
        }
        *response << (boost::format("HTTP/1.1 %1%\r\n") % code_repr).str()
                  << "Content-Type: application/json\r\n"
                  << "Access-Control-Allow-Origin: *\r\n"
                  << "Access-Control-Allow-Headers: *\r\n"
                  << "Access-Control-Allow-Methods: *\r\n"
                  << "Content-Length: " << dumped.length() << "\r\n\r\n"
                  << dumped;
    }
};

template <typename ResponseStream>
struct Make503
{
    std::shared_ptr<ResponseStream> response;
    nlohmann::json answer;

    void operator()()
    {
        auto r = answer.dump(2);
        *response << "HTTP/1.1 503 Service Unavailable\r\n"
                  << "Content-Type: application/json\r\n"
                  << "Access-Control-Allow-Origin: *\r\n"
                  << "Access-Control-Allow-Headers: *\r\n"
                  << "Access-Control-Allow-Methods: *\r\n"
                  << "Content-Length: " << r.length() << "\r\n\r\n"
                  << r;
    }
};



template <typename ResponseStream>
struct MakeCorsAnswer
{
    std::shared_ptr<ResponseStream> response;

    void operator()(std::string_view allowed_origin,
                    std::string_view allowed_headers,
                    std::string_view allowed_methods)
    {
        *response << "HTTP/1.1 200 OK\r\n"
                  << "Access-Control-Allow-Origin: " << allowed_origin << "\r\n"
                  << "Access-Control-Allow-Headers: " << allowed_headers << "\r\n"
                  << "Access-Control-Allow-Methods: " << allowed_methods << "\r\n"
                  << "Content-Length: 0\r\n"
                  << "Cache-Control: max-age=86400\r\n"
                  << "Keep-Alive: timeout=2, max=100\r\n"
                  << "Connection: keep-alive\r\n"
                  << std::endl ;
    }
};


const auto make_200 = [] (auto&& response, auto&& answer)
{
    using ResponseType = typename std::decay_t<decltype(response)>::element_type;
    using AnswerType = std::decay_t<decltype(answer)>;
    Make200<ResponseType, AnswerType> { std::forward<std::decay_t<decltype(response)>>(response),
                                        std::forward<std::decay_t<decltype(answer)>>(answer) }();
};

const auto make_4xx = [] (auto&& response, auto&& error, int code)
{
    using ResponseType = typename std::decay_t<decltype(response)>::element_type;
    using ErrorType = std::decay_t<decltype(error)>;
    Make4xx<ResponseType, ErrorType> { std::forward<std::decay_t<decltype(response)>>(response),
                                       std::forward<std::decay_t<decltype(error)>>(error) }(code);
};

const auto make_5xx = [] (auto&& response, auto&& error, int code)
{
    using ResponseType = typename std::decay_t<decltype(response)>::element_type;
    using ErrorType = std::decay_t<decltype(error)>;
    Make5xx<ResponseType, ErrorType> { std::forward<std::decay_t<decltype(response)>>(response),
                                       std::forward<std::decay_t<decltype(error)>>(error) }(code);
};

const auto make_503 = [] (auto&& response, auto&& asnwer)
{
    using ResponseType = typename std::decay_t<decltype(response)>::element_type;
    Make503<ResponseType> { std::forward<std::decay_t<decltype(response)>>(response),
                            std::forward<std::decay_t<decltype(asnwer)>>(asnwer) }();
};

const auto make_cors = [] (auto&& response,
                           std::string_view allowed_origin = "*",
                           std::string_view allowed_headers = "session_hash, content-type",
                           std::string_view allowed_methods = "POST, GET, OPTIONS")
{
    using ResponseType = typename std::decay_t<decltype(response)>::element_type;
    MakeCorsAnswer<ResponseType> { std::forward<std::decay_t<decltype(response)>>(response) }(
        allowed_origin, allowed_headers, allowed_methods
    );
};

}

#endif //CAMERAMANAGERCORE_RESPONSEBUILDER_HPP
