//
// Created by svakhreev on 05.04.17.
//


#include "SignalStatus.hpp"
#include <rttr/registration>

using namespace rttr;

namespace cameramanagercore::schemas::model
{

RTTR_REGISTRATION
{
    registration::enumeration<SignalStatus>("SignalStatus")
        (
            value("OnAdd",     SignalStatus::OnAdd),
            value("OnChange",  SignalStatus::OnChange),
            value("OnRemove",  SignalStatus::OnRemove)
        );
}

}