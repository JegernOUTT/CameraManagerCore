//
// Created by svakhreev on 29.03.17.
//

#include "Sessions.hpp"
#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::class_<Sessions>("Sessions")
        .constructor<>()
        .property("session_hash",     &Sessions::session_hash)
        .property("user_id",          &Sessions::user_id)
        .property("creation_time",    &Sessions::creation_time)
        .property("destruction_time", &Sessions::destruction_time);
}

}