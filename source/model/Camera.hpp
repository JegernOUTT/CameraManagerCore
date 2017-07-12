//
// Created by svakhreev on 29.12.16.
//

#ifndef CAMERAMANAGERCORE_CAMERA_HPP
#define CAMERAMANAGERCORE_CAMERA_HPP

#include <string>
#include "Common.hpp"

namespace cameramanagercore::model
{

enum class CameraInteraction
{
    Onvif = 1, Axis = 2
};


struct Camera
{
    std::string name;
    std::string url;
    std::string login;
    std::string password;
    CameraInteraction interaction_id;
    int idle_timeout_sec;
    std::string on_idle_action;
    int id;
};

}

#endif //CAMERAMANAGERCORE_CAMERA_HPP
