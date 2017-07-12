//
// Created by svakhreev on 07.02.17.
//

#ifndef CAMERAMANAGERCORE_SETTINGS_HPP
#define CAMERAMANAGERCORE_SETTINGS_HPP

#include <vector>
#include "Common.hpp"

namespace cameramanagercore::model
{

struct Settings
{
    std::vector<int> registration;
    int http_port;
    int ws_camera_port;
    int ws_model_port;
    int ws_events_port;
    int id;
};

}


#endif //CAMERAMANAGERCORE_SETTINGS_HPP
