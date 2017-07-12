//
// Created by svakhreev on 07.03.17.
//

#ifndef CAMERAMANAGERCORE_CAMERASTATUSINFORMATION_HPP
#define CAMERAMANAGERCORE_CAMERASTATUSINFORMATION_HPP

#include <string>
#include "Action.hpp"
#include "Common.hpp"

namespace onvifwrapper
{
    struct PTZStatus;
}

using SqliteDateTime = int64_t;

namespace cameramanagercore::model
{

struct Camera;
struct User;
struct Action;

struct CameraAction
{
    bool succeed;
    int camera_id;
    std::string session_hash;
    SqliteDateTime time;
    std::string action;
    int id;
};

struct CameraStatus
{
    int camera_id;
    PTZStatus ptz_status;
};

struct CameraInIdle
{
    int camera_id;
    bool in_idle;
};

}

#endif //CAMERAMANAGERCORE_CAMERASTATUSINFORMATION_HPP
