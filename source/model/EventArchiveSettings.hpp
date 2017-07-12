//
// Created by svakhreev on 06.04.17.
//

#ifndef CAMERAMANAGERCORE_EVENTARCHIVESETTINGS_HPP
#define CAMERAMANAGERCORE_EVENTARCHIVESETTINGS_HPP

#include <string>

#include "Common.hpp"

namespace cameramanagercore::model
{

struct EventArchiveSettings
{
    int camera_id;
    std::string event_categories;
    int buffer_seconds;
    int record_seconds;
    int id;
};

}

#endif //CAMERAMANAGERCORE_EVENTARCHIVESETTINGS_HPP
