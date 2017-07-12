//
// Created by svakhreev on 07.02.17.
//

#include "UserCameras.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::class_<UserCameras>("UserCameras")
        .constructor<>()
        .property("user_id",     &UserCameras::user_id)
        .property("camera_id",   &UserCameras::camera_id);
}

}
