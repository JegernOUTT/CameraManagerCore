//
// Created by svakhreev on 27.03.17.
//

#include "OperationResponse.hpp"
#include <rttr/registration>

using namespace rttr;

namespace cameramanagercore::schemas::model
{

RTTR_REGISTRATION
{
    registration::class_<OperationResponse>("OperationResponse")
        .constructor<>()
        .property("code",    &OperationResponse::code)
        .property("message", &OperationResponse::message);
}

}