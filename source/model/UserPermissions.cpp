//
// Created by svakhreev on 07.02.17.
//

#include "UserPermissions.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::class_<UserPermissions>("UserPermissions")
        .constructor<>()
        .property("user_id",         &UserPermissions::user_id)
        .property("permission_id",   &UserPermissions::permission_id);
}

}