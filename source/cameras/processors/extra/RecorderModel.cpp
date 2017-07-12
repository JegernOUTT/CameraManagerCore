//
// Created by svakhreev on 11.04.17.
//

#include "RecorderModel.hpp"

namespace cameramanagercore::cameras::processors::extra
{

using namespace nlohmann;

std::string operation_status_to_string(OperationStatus status)
{
    switch (status)
    {
        case OperationStatus::Ok:      return "Ok";
        case OperationStatus::Failed:  return "Failed";
        case OperationStatus::ConnectionFailed:  return "ConnectionFailed";
        default: return "";
    }
}

OperationStatus operation_status_from_string(std::string_view status)
{
    if (status == "Ok")
    {
        return OperationStatus::Ok;
    }
    else if (status == "ConnectionFailed")
    {
        return OperationStatus::ConnectionFailed;
    }
    else
    {
        return OperationStatus::Failed;
    }
}

std::string signal_status_to_string(SignalStatus status)
{
    switch (status)
    {
        case SignalStatus::OnAdd:      return "OnAdd";
        case SignalStatus::OnChange:   return "OnChange";
        case SignalStatus::OnRemove:   return "OnRemove";
        default: return "";
    }
}

SignalStatus signal_status_from_string(std::string_view status)
{
    if (status == "OnAdd")
    {
        return SignalStatus::OnAdd;
    }
    else if (status == "OnChange")
    {
        return SignalStatus::OnChange;
    }
    else
    {
        return SignalStatus::OnRemove;
    }
}

void to_json(json& j, const EventSettings& p)
{
    j = json{ {"camera_id",        p.camera_id},
              {"event_categories", p.event_categories},
              {"buffer_time",      p.buffer_time},
              {"record_time",      p.record_time} };
}

void from_json(const json& j, EventSettings& p)
{
    p.camera_id        = j.at("camera_id").get<int>();
    p.event_categories = j.at("event_categories").get<std::string>();
    p.buffer_time      = j.at("buffer_time").get<int>();
    p.record_time      = j.at("record_time").get<int>();
}


void to_json(json& j, const CameraInformation& p)
{
    j = json{ {"stream_url",    p.stream_url},
        j = json  {"websocket_url", p.websocket_url},
              {"archive_path",  p.archive_path} };
}

void from_json(const json& j, CameraInformation& p)
{
    p.stream_url    = j.at("stream_url").get<std::string>();
    p.archive_path  = j.at("archive_path").get<std::string>();
    p.websocket_url = j.at("websocket_url").get<std::string>();
}

void to_json(json& j, const Input& p)
{
    j = json{ {"event_settings",     p.event_settings},
              {"camera_information", p.camera_information} };
}

void from_json(const json& j, Input& p)
{
    p.event_settings     = j.at("event_settings").get<EventSettings>();
    p.camera_information = j.at("camera_information").get<CameraInformation>();
}

void to_json(json& j, const Output& p)
{
    j = json{ {"event_hash",  p.event_hash},
              {"status",      operation_status_to_string(p.status)} };
}

void from_json(const json& j, Output& p)
{
    p.event_hash = j.at("event_hash").get<std::string>();
    p.status     = operation_status_from_string(j.at("status").get<std::string>());
}

void to_json(json& j, const Event& p)
{
    j = json{ {"hash",                     p.hash},
              {"name",                     p.name},
              {"description",              p.description},
              {"camera_id",                p.camera_id},
              {"category",                 p.category},
              {"created_session_hash",     p.created_session_hash},
              {"created_time",             p.created_time},
              {"on_create_action",         p.on_create_action},
              {"processed",                p.processed},
              {"processed_time",           p.processed_time},
              {"acknowledged",             p.acknowledged},
              {"acknowledge_session_hash", p.acknowledge_session_hash},
              {"acknowledged_time",        p.acknowledged_time},
              {"on_acknowledge_action",    p.on_acknowledge_action} };
}

void from_json(const json& j, Event& p)
{
    p.hash                     = j.at("hash").get<std::string>();
    p.name                     = j.at("name").get<std::string>();
    p.description              = j.at("description").get<std::string>();
    p.camera_id                = j.at("camera_id").get<int>();
    p.category                 = j.at("category").get<std::string>();
    p.created_session_hash     = j.at("created_session_hash").get<std::string>();
    p.created_time             = j.at("created_time").get<std::string>();
    p.on_create_action         = j.at("on_create_action").get<std::string>();
    p.processed                = j.at("processed").get<bool>();
    p.processed_time           = j.at("processed_time").get<std::string>();
    p.acknowledged             = j.at("acknowledged").get<bool>();
    p.acknowledge_session_hash = j.at("acknowledge_session_hash").get<std::string>();
    p.acknowledged_time        = j.at("acknowledged_time").get<std::string>();
    p.on_acknowledge_action    = j.at("on_acknowledge_action").get<std::string>();
}

void from_json(const json& j, SignalEvent& p)
{
    p.data     = j.at("data").get<Event>();
    p.status   = signal_status_from_string(j.at("status").get<std::string>());
}

void to_json(json& j, const SignalEvent& p)
{
    j = json{ {"data",   p.data},
              {"status", signal_status_to_string(p.status)} };
}

}