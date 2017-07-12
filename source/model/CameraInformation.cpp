//
// Created by svakhreev on 09.03.17.
//

#include "CameraInformation.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::class_<CameraAction>("CameraAction")
        .constructor<>()
        .property("succeed",      &CameraAction::succeed)
        .property("camera_id",    &CameraAction::camera_id)
        .property("session_hash", &CameraAction::session_hash)
        .property("time",         &CameraAction::time)
        .property("action",       &CameraAction::action)
        .property("id",           &CameraAction::id);

    registration::class_<CameraStatus>("CameraStatus")
        .constructor<>()
        .property("camera_id",           &CameraStatus::camera_id)
        .property("ptz_status",          &CameraStatus::ptz_status);

    registration::class_<CameraInIdle>("CameraInIdle")
        .constructor<>()
        .property("camera_id",           &CameraInIdle::camera_id)
        .property("in_idle",             &CameraInIdle::in_idle);
}

}
