//
// Created by svakhreev on 08.02.17.
//

#ifndef CAMERAMANAGERCORE_AXIS_HPP
#define CAMERAMANAGERCORE_AXIS_HPP

#include <utility>
#include <tuple>
#include <vector>
#include <string>
#include <onvif_wrapper/OnvifPod.hpp>

namespace cameramanagercore::cameras::interactions
{

using namespace onvifwrapper;

using login = std::string;
using password = std::string;
using url = std::string;

struct Axis
{
    explicit Axis(login user_name, password user_password, url url) {  }

public:
    // Device methods
    std::tuple<onvifwrapper::Result, onvifwrapper::OnvifCapabilities>
    GetCapabilities() { return {}; }

    std::tuple<onvifwrapper::Result, std::string>
    SystemReboot() { return {}; };

    std::tuple<onvifwrapper::Result, onvifwrapper::DeviceInformation>
    GetDeviceInformation() { return {}; };


    // Media methods
    std::tuple<onvifwrapper::Result, onvifwrapper::Profile>
    GetMediaProfile(std::string profile_token) { return {}; };

    std::tuple<onvifwrapper::Result, std::vector<onvifwrapper::Profile>>
    GetMediaProfiles() { return {}; };

    std::tuple<onvifwrapper::Result, std::vector<onvifwrapper::VideoSource>>
    GetVideoSources() { return {}; };

    std::tuple<onvifwrapper::Result, onvifwrapper::VideoSourceConfiguration>
    GetVideoSourceConfiguration(std::string configuration_token) { return {}; };

    std::tuple<onvifwrapper::Result, std::vector<onvifwrapper::VideoSourceConfiguration>>
    GetVideoSourceConfigurations() { return {}; };

    std::tuple<onvifwrapper::Result, onvifwrapper::VideoSourceConfigurationOptions>
    GetVideoSourceConfigurationOptions(std::string configuration_token,
                                       std::string profile_token) { return {}; };

    std::tuple<onvifwrapper::Result, onvifwrapper::MediaUri>
    GetStreamUri(onvifwrapper::StreamType stream_type,
                 onvifwrapper::TransportProtocol transport_protocol) { return {}; };

    std::tuple<onvifwrapper::Result, onvifwrapper::MediaUri>
    GetSnapshotUri() { return {}; };

    onvifwrapper::Result
    StartMulticastStreaming() { return {}; };

    onvifwrapper::Result
    StopMulticastStreaming() { return {}; };

    std::tuple<onvifwrapper::Result, onvifwrapper::Capabilities>
    GetServiceCapabilities() { return {}; };


    // Ptz methods
    std::tuple<onvifwrapper::Result, onvifwrapper::PTZConfiguration>
    GetConfiguration(std::string ptz_configuration_token) { return {}; };

    std::tuple<onvifwrapper::Result, std::vector<onvifwrapper::PTZConfiguration>>
    GetConfigurations() { return {}; };

    std::tuple<onvifwrapper::Result, onvifwrapper::PTZConfigurationOptions>
    GetConfigurationOptions(std::string ptz_configuration_token) { return {}; };

    onvifwrapper::Result
    RelativeMove(onvifwrapper::PTZVector translation,
                 onvifwrapper::PTZSpeed speed) { return {}; };

    onvifwrapper::Result
    ContinuousMove(onvifwrapper::PTZSpeed speed,
                   std::string timeout) { return {}; };

    onvifwrapper::Result
    AbsoluteMove(onvifwrapper::PTZSpeed speed,
                 onvifwrapper::PTZVector position) { return {}; };

    onvifwrapper::Result
    Stop(bool pan_tilt,
         bool zoom) { return {}; };

    std::tuple<onvifwrapper::Result, std::vector<onvifwrapper::Preset>>
    GetPresets() { return {}; };

    onvifwrapper::Result
    GoToPreset(std::string preset_token,
               onvifwrapper::PTZSpeed speed) { return {}; };

    std::tuple<onvifwrapper::Result, std::string>
    ChangePreset(std::string preset_token,
                 std::string new_preset_name) { return {}; };

    std::tuple<onvifwrapper::Result, std::string>
    AddPreset(std::string new_preset_name) { return {}; };

    onvifwrapper::Result
    RemovePreset(std::string preset_token) { return {}; };

    onvifwrapper::Result
    GoToHome(onvifwrapper::PTZSpeed speed) { return {}; };

    onvifwrapper::Result
    SetHome() { return {}; };

    std::tuple<onvifwrapper::Result, onvifwrapper::PTZStatus>
    GetStatus() { return {}; };

    std::tuple<onvifwrapper::Result, onvifwrapper::PTZCapabilitiesInformation>
    GetPtzServiceCapabilities() { return {}; };

    std::tuple<onvifwrapper::Result, onvifwrapper::PTZNode>
    GetNode(std::string node_token) { return {}; };

    std::tuple<onvifwrapper::Result, std::vector<onvifwrapper::PTZNode>>
    GetNodes() { return {}; };
};

}

#endif //CAMERAMANAGERCORE_AXIS_HPP
