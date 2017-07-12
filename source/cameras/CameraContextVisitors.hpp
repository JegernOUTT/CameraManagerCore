//
// Created by svakhreev on 07.03.17.
//

#ifndef CAMERAMANAGERCORE_CAMERACONTEXTVISITORS_HPP
#define CAMERAMANAGERCORE_CAMERACONTEXTVISITORS_HPP

#include <memory>
#include <chrono>
#include <boost/format.hpp>

#include <nlohmann/json.hpp>

#include "../model/CameraInformation.hpp"
#include "../model/ModelContext.hpp"
#include "../signals/SignalContext.hpp"

namespace cameramanagercore::cameras
{
using namespace cameramanagercore::signals;
using namespace onvifwrapper;

using SignalContextPtr = std::shared_ptr<SignalsContext>;
using ModelPtr = std::shared_ptr<cameramanagercore::model::Model>;

template<typename TimeDuration>
auto Now()
{
    using namespace std::chrono;
    return duration_cast<TimeDuration>(system_clock::now().time_since_epoch()).count();
}

// interaction, signal_context, camera_action, other_params
/*
 *
 *
 * PTZ
 *
 *
 *
 * */
const auto RelativeMove = [] (auto interaction,
                              SignalContextPtr signals_context,
                              cameramanagercore::model::CameraAction camera_action,
                              std::tuple<ModelPtr, PTZVector, PTZSpeed> t)
{
    auto& [model_context, translation, speed] = t;
    auto status = interaction->RelativeMove(translation, speed);
    camera_action.time = Now<std::chrono::seconds>();
    camera_action.action = nlohmann::json(cameramanagercore::model::Action
                                              { cameramanagercore::model::ActionType::RelativeMove, {},
                                                {translation, speed} }).dump();

    model_context->Add(std::make_shared<cameramanagercore::model::CameraAction>(camera_action));

    return status;
};
const auto ContinuousMove = [] (auto interaction,
                                SignalContextPtr signals_context,
                                cameramanagercore::model::CameraAction camera_action,
                                std::tuple<ModelPtr, PTZSpeed, int> t)
{
    auto& [model_context, speed, timeout] = t;
    auto status = interaction->ContinuousMove(speed, (boost::format("PT%1%S") % timeout).str());
    camera_action.time = Now<std::chrono::seconds>();
    camera_action.action = nlohmann::json(cameramanagercore::model::Action
                                              { cameramanagercore::model::ActionType::ContinuousMove,
                                                {}, {}, {speed, timeout} }).dump();

    model_context->Add(std::make_shared<cameramanagercore::model::CameraAction>(camera_action));

    return status;
};
const auto AbsoluteMove = [] (auto interaction,
                              SignalContextPtr signals_context,
                              cameramanagercore::model::CameraAction camera_action,
                              std::tuple<ModelPtr, PTZSpeed, PTZVector> t)
{
    auto& [model_context, speed, position] = t;
    auto status = interaction->AbsoluteMove(speed, position);
    camera_action.time = Now<std::chrono::seconds>();
    camera_action.action = nlohmann::json(cameramanagercore::model::Action
                                              { cameramanagercore::model::ActionType::AbsoluteMove,
                                                {}, {}, {}, {speed, position} }).dump();

    model_context->Add(std::make_shared<cameramanagercore::model::CameraAction>(camera_action));

    return status;
};
const auto Stop = [] (auto interaction,
                      SignalContextPtr signals_context,
                      cameramanagercore::model::CameraAction camera_action,
                      std::tuple<bool, bool> t)
{
    auto& [pan_tilt, zoom] = t;
    return interaction->Stop(pan_tilt, zoom);;
};
const auto GetPresets = [] (auto interaction,
                            SignalContextPtr signals_context,
                            cameramanagercore::model::CameraAction camera_action)
{
    return interaction->GetPresets();
};
const auto GoToPreset = [] (auto interaction,
                            SignalContextPtr signals_context,
                            cameramanagercore::model::CameraAction camera_action,
                            std::tuple<ModelPtr, string> t)
{
    auto& [model_context, preset_token] = t;
    auto status = interaction->GoToPreset(preset_token, {{1., 1.}, {1.}});
    camera_action.time = Now<std::chrono::seconds>();
    camera_action.action = nlohmann::json(cameramanagercore::model::Action
                                              { cameramanagercore::model::ActionType::GoToPreset,
                                                { preset_token } }).dump();

    model_context->Add(std::make_shared<cameramanagercore::model::CameraAction>(camera_action));

    return status;
};
const auto ChangePreset = [] (auto interaction,
                              SignalContextPtr signals_context,
                              cameramanagercore::model::CameraAction camera_action,
                              std::tuple<string, string> t)
{
    auto& [preset_token, new_preset_name] = t;
    return interaction->ChangePreset(preset_token, new_preset_name);
};
const auto AddPreset = [] (auto interaction,
                           SignalContextPtr signals_context,
                           cameramanagercore::model::CameraAction camera_action,
                           std::tuple<string> t)
{
    auto& [new_preset_name] = t;
    return interaction->AddPreset(new_preset_name);
};
const auto RemovePreset = [] (auto interaction,
                              SignalContextPtr signals_context,
                              cameramanagercore::model::CameraAction camera_action,
                              std::tuple<string> t)
{
    auto& [preset_token] = t;
    return interaction->RemovePreset(preset_token);
};
const auto GoToHome = [] (auto interaction,
                          SignalContextPtr signals_context,
                          cameramanagercore::model::CameraAction camera_action,
                          std::tuple<ModelPtr> t)
{
    auto& [model_context] = t;
    auto status = interaction->GoToHome({{1., 1.}, {1.}});
    camera_action.time = Now<std::chrono::seconds>();
    camera_action.action = nlohmann::json(cameramanagercore::model::Action
                                              { cameramanagercore::model::ActionType::GoToHome,
                                                {} }).dump();

    model_context->Add(std::make_shared<cameramanagercore::model::CameraAction>(camera_action));

    return status;
};
const auto SetHome = [] (auto interaction,
                         SignalContextPtr signals_context,
                         cameramanagercore::model::CameraAction camera_action)
{
    return interaction->SetHome();
};
const auto GetStatus = [] (auto interaction,
                           SignalContextPtr signals_context,
                           cameramanagercore::model::CameraAction camera_action)
{
    return interaction->GetStatus();
};
const auto GetCapabilities = [] (auto interaction,
                                 SignalContextPtr signals_context,
                                 cameramanagercore::model::CameraAction camera_action)
{
    return interaction->GetPtzServiceCapabilities();
};
const auto GetNodes = [] (auto interaction,
                          SignalContextPtr signals_context,
                          cameramanagercore::model::CameraAction camera_action)
{
    return interaction->GetNodes();
};


/*
 *
 *
 * Device
 *
 *
 *
 * */
const auto CameraReboot = [] (auto interaction,
                              SignalContextPtr signals_context,
                              cameramanagercore::model::CameraAction camera_action)
{
    return interaction->SystemReboot();
};
const auto GetDeviceInformation = [] (auto interaction,
                                      SignalContextPtr signals_context,
                                      cameramanagercore::model::CameraAction camera_action)
{
    return interaction->GetDeviceInformation();
};


/*
 *
 *
 * Media
 *
 *
 *
 * */
const auto GetMediaProfiles = [] (auto interaction,
                                  SignalContextPtr signals_context,
                                  cameramanagercore::model::CameraAction camera_action)
{
    return interaction->GetMediaProfiles();
};
const auto GetStreamUri = [] (auto interaction,
                              SignalContextPtr signals_context,
                              cameramanagercore::model::CameraAction camera_action,
                              std::tuple<StreamType, TransportProtocol> t)
{
    auto& [stream_type, transport_protocol] = t;
    return interaction->GetStreamUri(stream_type, transport_protocol);
};
const auto StartMulticast = [] (auto interaction,
                                SignalContextPtr signals_context,
                                cameramanagercore::model::CameraAction camera_action)
{
    return interaction->StartMulticastStreaming();
};
const auto StopMulticast = [] (auto interaction,
                               SignalContextPtr signals_context,
                               cameramanagercore::model::CameraAction camera_action)
{
    return interaction->StopMulticastStreaming();
};


/*
 *
 *
 * From action
 *
 *
 *
 * */
const auto FromAction = [] (auto interaction,
                            cameramanagercore::model::Action action)
{
    switch (action.action)
    {
        case cameramanagercore::model::ActionType::RelativeMove:
        {
            return interaction->RelativeMove(action.relative_move_information.translation,
                                             action.relative_move_information.speed);
        }

        case cameramanagercore::model::ActionType::ContinuousMove:
        {
            auto timeout = (boost::format("PT%1%S") % action.continuous_move_information.timeout).str();
            return interaction->ContinuousMove(action.continuous_move_information.speed,
                                               timeout);
        }

        case cameramanagercore::model::ActionType::AbsoluteMove:
        {
            return interaction->AbsoluteMove(action.absolute_move_information.speed,
                                             action.absolute_move_information.position);
        }

        case cameramanagercore::model::ActionType::GoToPreset:
        {
            return interaction->GoToPreset(action.preset_information.preset_token,
                                           {{1., 1.}, {1.}});
        }

        case cameramanagercore::model::ActionType::GoToHome:
        {
            return interaction->GoToHome({{1., 1.}, {1.}});
        }

        default:
        {
            return Result {};
        }
    }
};

}

#endif //CAMERAMANAGERCORE_CAMERACONTEXTVISITORS_HPP
