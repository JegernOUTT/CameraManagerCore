//
// Created by svakhreev on 06.03.17.
//

#include "ContextMisc.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{
using namespace rttr;

RTTR_REGISTRATION
{
    registration::enumeration<OperationStatus>("OperationStatus")
        (
            value("Ok",           OperationStatus::Ok),
            value("SameId",       OperationStatus::SameId),
            value("SameBody",     OperationStatus::SameBody),
            value("NotFound",     OperationStatus::NotFound),
            value("UnknownError", OperationStatus::UnknownError)
        );
}
}