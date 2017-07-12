//
// Created by svakhreev on 07.02.17.
//

#ifndef CAMERAMANAGERCORE_EVENT_HPP
#define CAMERAMANAGERCORE_EVENT_HPP

#include <string>

#include "Common.hpp"

namespace cameramanagercore::model
{

enum class EventStatus { Initial, Processed, Acknowledged, ProcessedAndAcknowledged, HasVideo };
using SqliteDateTime = int64_t;

struct Event
{
    std::string hash;
    std::string name;
    std::string description;
    int camera_id;
    std::string category;

    std::string created_session_hash;
    SqliteDateTime created_time;
    std::string on_create_action;

    bool processed;
    SqliteDateTime processed_time;

    bool acknowledged;
    std::string acknowledge_session_hash;
    SqliteDateTime acknowledged_time;
    std::string on_acknowledge_action;

    bool has_video;
};

}


#endif //CAMERAMANAGERCORE_EVENT_HPP
