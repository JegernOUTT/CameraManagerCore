//
// Created by svakhreev on 15.03.17.
//

#include "SignalGenerator.hpp"

namespace cameramanagercore::signals
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::enumeration<DataStatus>("DataStatus")
        (
            value("Added",   DataStatus::Added),
            value("Changed", DataStatus::Changed),
            value("Removed", DataStatus::Removed)
        );
}

}