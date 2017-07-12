//
// Created by svakhreev on 07.03.17.
//

#include "Action.hpp"

#include <rttr/registration>

namespace cameramanagercore::model
{

using namespace rttr;

RTTR_REGISTRATION
{
    registration::enumeration<ActionType>("ActionType")
        (
            value("RelativeMove",   ActionType::RelativeMove),
            value("ContinuousMove", ActionType::ContinuousMove),
            value("AbsoluteMove",   ActionType::AbsoluteMove),
            value("GoToPreset",     ActionType::GoToPreset),
            value("GoToHome",       ActionType::GoToHome)
        );

    registration::class_<PresetInformation>("PresetInformation")
        .constructor<>()
        .property("preset_token",                &PresetInformation::preset_token);

    registration::class_<RelativeMoveInformation>("RelativeMoveInformation")
        .constructor<>()
        .property("translation",                 &RelativeMoveInformation::translation)
        .property("speed",                       &RelativeMoveInformation::speed);

    registration::class_<ContinuousMoveInformation>("ContinuousMoveInformation")
        .constructor<>()
        .property("speed",                       &ContinuousMoveInformation::speed)
        .property("timeout",                     &ContinuousMoveInformation::timeout);

    registration::class_<AbsoluteMoveInformation>("AbsoluteMoveInformation")
        .constructor<>()
        .property("speed",                       &AbsoluteMoveInformation::speed)
        .property("position",                    &AbsoluteMoveInformation::position);

    registration::class_<Action>("Action")
        .constructor<>()
        .property("action",                      &Action::action)
        .property("preset_information",          &Action::preset_information)
        .property("relative_move_information",   &Action::relative_move_information)
        .property("continuous_move_information", &Action::continuous_move_information)
        .property("absolute_move_information",   &Action::absolute_move_information);
}

}
