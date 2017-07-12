//
// Created by svakhreev on 06.04.17.
//

#include "EventArchiveSettings.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::class_<EventArchiveSettings>("EventArchiveSettings")
        .constructor<>()
        .property("camera_id",        &EventArchiveSettings::camera_id)
        .property("event_categories", &EventArchiveSettings::event_categories)
        .property("buffer_seconds",   &EventArchiveSettings::buffer_seconds)
        .property("record_seconds",   &EventArchiveSettings::record_seconds)
        .property("id",               &EventArchiveSettings::id);
}

}