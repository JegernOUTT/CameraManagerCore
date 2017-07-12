//
// Created by svakhreev on 07.02.17.
//

#include "Permission.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::class_<Permission>("Permission")
        .constructor<>()
        .property("name",           &Permission::name)
        .property("regex_url",      &Permission::regex_url)
        .property("id",             &Permission::id);
}

}