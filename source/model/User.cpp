//
// Created by svakhreev on 29.12.16.
//

#include "User.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::class_<User>("User")
        .constructor<>()
        .property("name",             &User::name)
        .property("password",         &User::password)
        .property("event_categories", &User::event_categories)
        .property("id",               &User::id);
}

}
