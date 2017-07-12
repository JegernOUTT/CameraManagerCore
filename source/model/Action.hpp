//
// Created by svakhreev on 07.03.17.
//

#ifndef CAMERAMANAGERCORE_ACTION_HPP
#define CAMERAMANAGERCORE_ACTION_HPP

#include <string>
#include <onvif_wrapper/OnvifPod.hpp>
#include "Common.hpp"

namespace cameramanagercore::model
{

using namespace onvifwrapper;

enum class ActionType
{
    RelativeMove, ContinuousMove, AbsoluteMove,
    GoToPreset, GoToHome
};

struct PresetInformation
{
    std::string preset_token;
};

struct RelativeMoveInformation
{
    PTZVector translation;
    PTZSpeed speed;
};

struct ContinuousMoveInformation
{
    PTZSpeed speed;
    int timeout;
};

struct AbsoluteMoveInformation
{
    PTZSpeed speed;
    PTZVector position;
};

struct Action
{
    ActionType action;
    PresetInformation preset_information;
    RelativeMoveInformation relative_move_information;
    ContinuousMoveInformation continuous_move_information;
    AbsoluteMoveInformation absolute_move_information;
};

}


#endif //CAMERAMANAGERCORE_ACTION_HPP
