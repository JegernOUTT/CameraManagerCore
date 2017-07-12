//
// Created by svakhreev on 05.04.17.
//

#ifndef CAMERAMANAGERCORE_SIGNALSTATUS_HPP
#define CAMERAMANAGERCORE_SIGNALSTATUS_HPP

#include <string>
#include "Parcers.hpp"

namespace cameramanagercore::schemas::model
{

enum class SignalStatus
{
    OnAdd, OnChange, OnRemove
};

}

#endif //CAMERAMANAGERCORE_SIGNALSTATUS_HPP
