//
// Created by svakhreev on 07.04.17.
//

#ifndef PROJECT_MODEL_HPP
#define PROJECT_MODEL_HPP

#include <string>
#include <experimental/string_view>
#include <nlohmann/json.hpp>

namespace camerarecorder
{

struct EventSettings
{
    int camera_id;
    std::string event_categories;
    int buffer_time;
    int record_time;
};

struct CameraInformation
{
    std::string stream_url;
    std::string archive_path;
    std::string websocket_url;
};

struct Input
{
    EventSettings event_settings;
    CameraInformation camera_information;
};

enum class OperationStatus
{
    Ok, Failed, ConnectionFailed
};

struct Output
{
    std::string event_hash;
    OperationStatus status;
};

using SqliteDateTime = int64_t;

struct Event
{
    std::string hash;
    std::string name;
    std::string description;
    int camera_id;
    std::string category;

    std::string created_session_hash;
    std::string created_time;
    std::string on_create_action;

    bool processed;
    std::string processed_time;

    bool acknowledged;
    std::string acknowledge_session_hash;
    std::string acknowledged_time;
    std::string on_acknowledge_action;
};

enum class SignalStatus
{
    OnAdd, OnChange, OnRemove
};

struct SignalEvent
{
    Event data;
    SignalStatus status;
};

void to_json(nlohmann::json& j, const EventSettings& p);
void from_json(const nlohmann::json& j, EventSettings& p);

void to_json(nlohmann::json& j, const CameraInformation& p);
void from_json(const nlohmann::json& j, CameraInformation& p);

void to_json(nlohmann::json& j, const Input& p);
void from_json(const nlohmann::json& j, Input& p);

void to_json(nlohmann::json& j, const Output& p);
void from_json(const nlohmann::json& j, Output& p);

void to_json(nlohmann::json& j, const Event& p);
void from_json(const nlohmann::json& j, Event& p);

void to_json(nlohmann::json& j, const SignalEvent& p);
void from_json(const nlohmann::json& j, SignalEvent& p);

}

#endif //PROJECT_MODEL_HPP
