//
// Created by svakhreev on 29.12.16.
//

#include "Camera.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::enumeration<CameraInteraction>("CameraInteraction")
        (
            value("Onvif", CameraInteraction::Onvif),
            value("Axis",  CameraInteraction::Axis)
        );

    registration::class_<Camera>("Camera")
        .constructor<>()
        .property("name",             &Camera::name)
        .property("url",              &Camera::url)
        .property("login",            &Camera::login)
        .property("password",         &Camera::password)
        .property("interaction_id",   &Camera::interaction_id)
        .property("idle_timeout_sec", &Camera::idle_timeout_sec)
        .property("on_idle_action",   &Camera::on_idle_action)
        .property("id",               &Camera::id);
}

}