//
// Created by svakhreev on 07.02.17.
//

#include "Event.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::enumeration<EventStatus>("EventStatus")
        (
            value("Initial",                  EventStatus::Initial),
            value("Processed",                EventStatus::Processed),
            value("Acknowledged",             EventStatus::Acknowledged),
            value("ProcessedAndAcknowledged", EventStatus::ProcessedAndAcknowledged)
        );

    registration::class_<Event>("Event")
        .constructor<>()
        .property("hash",                     &Event::hash)
        .property("name",                     &Event::name)
        .property("description",              &Event::description)
        .property("camera_id",                &Event::camera_id)
        .property("category",                 &Event::category)

        .property("created_session_hash",     &Event::created_session_hash)
        .property("created_time",             &Event::created_time)
        .property("on_create_action",         &Event::on_create_action)

        .property("processed",                &Event::processed)
        .property("processed_time",           &Event::processed_time)

        .property("acknowledged",             &Event::acknowledged)
        .property("acknowledge_session_hash", &Event::acknowledge_session_hash)
        .property("acknowledged_time",        &Event::acknowledged_time)
        .property("on_acknowledge_action",    &Event::on_acknowledge_action)

        .property("has_video",                &Event::has_video);
}

}