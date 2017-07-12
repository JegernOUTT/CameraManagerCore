//
// Created by svakhreev on 07.02.17.
//

#ifndef CAMERAMANAGERCORE_ISSCONNECTION_HPP
#define CAMERAMANAGERCORE_ISSCONNECTION_HPP

#include <string>
#include <map>
#include <boost/format.hpp>
#include <unordered_map>
#include <optional>

#include <simple-web-server/client_http.hpp>
#include <nlohmann/json.hpp>

#include "../utility/LoggerSettings.hpp"

namespace cameramanagercore::iss
{

using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

enum class ApiPath
{
    GetCamerasAllInformation, GetCamerasConnectionInformation,
    GetCameraRtspInformation, GetUsersInformation, GetCameraEvents
};

template <typename Logger>
class IssConnection
{
public:
    IssConnection(std::string host,
                  std::string login,
                  std::string password) :
        _client(host),
        _paths { { ApiPath::GetCamerasAllInformation,        "/api/v1/cameras" },
                 { ApiPath::GetCamerasConnectionInformation, "/api/v2/cameras" },
                 { ApiPath::GetCameraRtspInformation,        "/api/v2/cameras/%1%/rtsp" },
                 { ApiPath::GetUsersInformation,             "/api/v1/persons" },
                 { ApiPath::GetCameraEvents,                 "/api/v2/cameras/%1%/protocol?start_time=%2%&max_count=%3%" }}
    {
    }

    std::optional<nlohmann::json> SendRequest(ApiPath path)
    {
        LOG(info) << "Send request to ISS";
        try
        {
            auto response = _client.request("GET", _paths[path]);
            if (response->status_code[0] == '4') return {};
            return response ? std::make_optional(nlohmann::json::parse(response->content)) : std::optional<nlohmann::json> {};
        }
        catch (...)
        {
            LOG(error) << "Some error in sending request to ISS or parsing answer";
            return std::optional<nlohmann::json> {};
        }
    }

    std::optional<nlohmann::json> SendRequest(ApiPath path, nlohmann::json j)
    {
        LOG(info) << "Send request to ISS";
        try
        {
            auto response = _client.request("GET", _paths[path], j.dump());
            if (response->status_code[0] == '4') return {};
            return response ? std::make_optional(nlohmann::json::parse(response->content)) : std::optional<nlohmann::json> {};
        }
        catch (...)
        {
            LOG(error) << "Some error in sending request to ISS or parsing answer";
            return std::optional<nlohmann::json> {};
        }
    }

    template <typename... Args>
    std::optional<nlohmann::json> SendRequest(ApiPath path, Args&&... args)
    {
        LOG(info) << "Send request to ISS";
        try
        {
            auto response = _client.request("GET", (boost::format(_paths[path]) % ... % args).str());
            if (response->status_code[0] == '4') return {};
            return response ? std::make_optional(nlohmann::json::parse(response->content)) : std::optional<nlohmann::json> {};
        }
        catch (...)
        {
            LOG(error) << "Some error in sending request to ISS or parsing answer";
            return std::optional<nlohmann::json> {};
        }
    }

private:
    HttpClient _client;
    std::unordered_map<ApiPath, std::string> _paths;
};

}

#endif //CAMERAMANAGERCORE_ISSCONNECTION_HPP
