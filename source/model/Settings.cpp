//
// Created by svakhreev on 07.02.17.
//

#include "Settings.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::class_<Settings>("Settings")
        .constructor<>()
        .property("registration",   &Settings::registration)
        .property("http_port",      &Settings::http_port)
        .property("ws_camera_port", &Settings::ws_camera_port)
        .property("ws_model_port",  &Settings::ws_model_port)
        .property("ws_events_port", &Settings::ws_events_port)
        .property("id",             &Settings::id);
}

}
