//
// Created by svakhreev on 10.03.17.
//

#include "CameraLockFrame.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::class_<CameraLockFrame>("CameraLockFrame")
        .constructor<>()
        .property("locked",      &CameraLockFrame::locked)
        .property("camera_id",   &CameraLockFrame::camera_id)
        .property("user_id",     &CameraLockFrame::user_id)
        .property("locked_time", &CameraLockFrame::locked_time)
        .property("timeout",     &CameraLockFrame::timeout)
        .property("lock_hash",   &CameraLockFrame::lock_hash);
}

}