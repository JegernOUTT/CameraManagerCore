//
// Created by svakhreev on 15.02.17.
//

#include <onvif_wrapper/OnvifPod.hpp>
#include <rttr/registration>

using namespace onvifwrapper;
using namespace rttr;

RTTR_REGISTRATION
{
    registration::enumeration<OnMode>("OnMode")
        (
            value("OFF", OnMode::OFF),
            value("ON", OnMode::ON)
        );
    registration::enumeration<ExpMode>("ExpMode")
        (
            value("AUTO", ExpMode::AUTO),
            value("MANUAL", ExpMode::MANUAL)
        );
    registration::enumeration<MoveStatus>("MoveStatus")
        (
            value("IDLE", MoveStatus::IDLE),
            value("MOVING", MoveStatus::MOVING),
            value("UNKNOWN", MoveStatus::UNKNOWN)
        );
    registration::enumeration<VideoEncoding>("VideoEncoding")
        (
            value("H264", VideoEncoding::H264),
            value("JPEG", VideoEncoding::JPEG),
            value("MPEG4", VideoEncoding::MPEG4)
        );
    registration::enumeration<Mpeg4Profile>("Mpeg4Profile")
        (
            value("ASP", Mpeg4Profile::ASP),
            value("SP", Mpeg4Profile::SP)
        );
    registration::enumeration<H264Profile>("H264Profile")
        (
            value("BASELINE", H264Profile::BASELINE),
            value("HIGH", H264Profile::HIGH),
            value("MAIN", H264Profile::MAIN),
            value("EXTENDED", H264Profile::EXTENDED)
        );
    registration::enumeration<StreamType>("StreamType")
        (
            value("RTP_MULTICAST", StreamType::RTP_MULTICAST),
            value("RTP_UNICAST", StreamType::RTP_UNICAST)
        );
    registration::enumeration<TransportProtocol>("TransportProtocol")
        (
            value("HTTP", TransportProtocol::HTTP),
            value("RTSP", TransportProtocol::RTSP),
            value("TCP", TransportProtocol::TCP),
            value("UDP", TransportProtocol::UDP)
        );

    registration::class_<AnalyticsCapabilities>("AnalyticsCapabilities")
        .property("address", &AnalyticsCapabilities::address)
        .property("rule_support", &AnalyticsCapabilities::rule_support)
        .property("analytics_module_support", &AnalyticsCapabilities::analytics_module_support);

    registration::class_<NetworkCapabilities>("NetworkCapabilities")
        .property("ip_filter", &NetworkCapabilities::ip_filter)
        .property("zero_configuration", &NetworkCapabilities::zero_configuration)
        .property("ip_version6", &NetworkCapabilities::ip_version6)
        .property("dyn_dns", &NetworkCapabilities::dyn_dns);

    registration::class_<SystemCapabilities>("SystemCapabilities")
        .property("discovery_resolve", &SystemCapabilities::discovery_resolve)
        .property("discovery_bye", &SystemCapabilities::discovery_bye)
        .property("remote_discovery", &SystemCapabilities::remote_discovery)
        .property("system_backup", &SystemCapabilities::system_backup)
        .property("system_logging", &SystemCapabilities::system_logging)
        .property("firmware_upgrade", &SystemCapabilities::firmware_upgrade);

    registration::class_<IOCapabilities>("IOCapabilities")
        .property("input_connectors", &IOCapabilities::input_connectors)
        .property("relay_outputs", &IOCapabilities::relay_outputs);

    registration::class_<SecurityCapabilities>("SecurityCapabilities")
        .property("tls1_x002_e1", &SecurityCapabilities::tls1_x002_e1)
        .property("tls1_x002_e2", &SecurityCapabilities::tls1_x002_e2)
        .property("onboard_key_generation", &SecurityCapabilities::onboard_key_generation)
        .property("access_policy_config", &SecurityCapabilities::access_policy_config)
        .property("x_x002_e509_token", &SecurityCapabilities::x_x002_e509_token)
        .property("saml_token", &SecurityCapabilities::saml_token)
        .property("kerberos_token", &SecurityCapabilities::kerberos_token)
        .property("rel_token", &SecurityCapabilities::rel_token);

    registration::class_<EventCapabilities>("EventCapabilities")
        .property("address", &EventCapabilities::address)
        .property("ws_subscription_policy_support", &EventCapabilities::ws_subscription_policy_support)
        .property("ws_pull_point_support", &EventCapabilities::ws_pull_point_support)
        .property("ws_pausable_subscription_manager_interface_support", &EventCapabilities::ws_pausable_subscription_manager_interface_support);

    registration::class_<ImagingCapabilities>("ImagingCapabilities")
        .property("address", &AnalyticsCapabilities::address);

    registration::class_<RealTimeStreamingCapabilities>("RealTimeStreamingCapabilities")
        .property("rtp_multicast", &RealTimeStreamingCapabilities::rtp_multicast)
        .property("rtp_uscoretcp", &RealTimeStreamingCapabilities::rtp_uscoretcp)
        .property("rtp_uscorertsp_uscoretcp", &RealTimeStreamingCapabilities::rtp_uscorertsp_uscoretcp);

    registration::class_<PTZCapabilities>("PTZCapabilities")
        .property("address", &PTZCapabilities::address);

    registration::class_<DeviceCapabilities>("DeviceCapabilities")
        .property("address", &DeviceCapabilities::address)
        .property("network", &DeviceCapabilities::network)
        .property("system", &DeviceCapabilities::system)
        .property("io", &DeviceCapabilities::io)
        .property("rule_support", &DeviceCapabilities::security);

    registration::class_<MediaCapabilities>("MediaCapabilities")
        .property("address", &MediaCapabilities::address)
        .property("streaming_capabilities", &MediaCapabilities::streaming_capabilities);

    registration::class_<OnvifCapabilities>("OnvifCapabilities")
        .property("analytics", &OnvifCapabilities::analytics)
        .property("device", &OnvifCapabilities::device)
        .property("events", &OnvifCapabilities::events)
        .property("imaging", &OnvifCapabilities::imaging)
        .property("media", &OnvifCapabilities::media)
        .property("ptz", &OnvifCapabilities::ptz);

    registration::class_<Result>("Result")
        .property("is_error", &Result::is_error)
        .property("error_code", &Result::error_code)
        .property("soap_error", &Result::soap_error)
        .property("full_error", &Result::full_error);

    registration::class_<DeviceInformation>("DeviceInformation")
        .property("firmaware", &DeviceInformation::firmaware)
        .property("hardware", &DeviceInformation::hardware)
        .property("manufacture", &DeviceInformation::manufacture)
        .property("model", &DeviceInformation::model)
        .property("serial", &DeviceInformation::serial);

    registration::class_<BackLightCompensation>("BackLightCompensation")
        .property("level", &BackLightCompensation::level)
        .property("mode", &BackLightCompensation::mode);

    registration::class_<Exposure>("Exposure")
        .property("mode", &Exposure::mode)
        .property("exposure_time", &Exposure::exposure_time)
        .property("gain", &Exposure::gain)
        .property("iris", &Exposure::iris)
        .property("max_exposure_time", &Exposure::max_exposure_time)
        .property("min_gain", &Exposure::min_gain)
        .property("max_iris", &Exposure::max_iris);

    registration::class_<ImageSettings>("ImageSettings")
        .property("back_light_compensation", &ImageSettings::back_light_compensation)
        .property("brightness", &ImageSettings::brightness)
        .property("color_saturation", &ImageSettings::color_saturation)
        .property("sharpness", &ImageSettings::sharpness)
        .property("contrast", &ImageSettings::contrast)
        .property("exposure", &ImageSettings::exposure);

    registration::class_<BackLightCompensationRange>("BackLightCompensationRange")
        .property("level_range", &BackLightCompensationRange::level_range)
        .property("mode_range", &BackLightCompensationRange::mode_range);

    registration::class_<ExposureRange>("ExposureRange")
        .property("mode_range", &ExposureRange::mode_range)
        .property("exposure_time_range", &ExposureRange::exposure_time_range)
        .property("gain_range", &ExposureRange::gain_range)
        .property("iris_range", &ExposureRange::iris_range)
        .property("max_exposure_time_range", &ExposureRange::max_exposure_time_range)
        .property("min_gain_range", &ExposureRange::min_gain_range)
        .property("max_iris_range", &ExposureRange::max_iris_range);

    registration::class_<ImageOptions>("ImageOptions")
        .property("back_light_compensation_range", &ImageOptions::back_light_compensation_range)
        .property("brightness", &ImageOptions::brightness)
        .property("color_saturation", &ImageOptions::color_saturation)
        .property("sharpness", &ImageOptions::sharpness)
        .property("contrast", &ImageOptions::contrast)
        .property("exposure", &ImageOptions::exposure);

    registration::class_<AbsoluteFocus>("AbsoluteFocus")
        .property("position", &AbsoluteFocus::position)
        .property("speed", &AbsoluteFocus::speed);

    registration::class_<RelativeFocus>("RelativeFocus")
        .property("distance", &RelativeFocus::distance)
        .property("speed", &RelativeFocus::speed);

    registration::class_<ContinuousFocus>("ContinuousFocus")
        .property("speed", &ContinuousFocus::speed);

    registration::class_<FocusMove>("FocusMove")
        .property("absolute_move", &FocusMove::absolute_move)
        .property("relative_move", &FocusMove::relative_move)
        .property("continuous_move", &FocusMove::continuous_move);

    registration::class_<AbsoluteFocusRange>("AbsoluteFocusRange")
        .property("position", &AbsoluteFocusRange::position)
        .property("speed", &AbsoluteFocusRange::speed);

    registration::class_<RelativeFocusRange>("RelativeFocusRange")
        .property("distance", &RelativeFocusRange::distance)
        .property("speed", &RelativeFocusRange::speed);

    registration::class_<ContinuousFocusRange>("ContinuousFocusRange")
        .property("speed", &ContinuousFocusRange::speed);

    registration::class_<FocusStatus>("FocusStatus")
        .property("position", &FocusStatus::position)
        .property("move_status", &FocusStatus::move_status)
        .property("error", &FocusStatus::error);

    registration::class_<ImagingStatus>("ImagingStatus")
        .property("focus_status", &ImagingStatus::focus_status);

    registration::class_<ImageCapabilities>("ImageCapabilities")
        .property("image_stabilization", &ImageCapabilities::image_stabilization)
        .property("presets", &ImageCapabilities::presets);

    registration::class_<IntRectangle>("IntRectangle")
        .property("x", &IntRectangle::x)
        .property("y", &IntRectangle::y)
        .property("width", &IntRectangle::width)
        .property("height", &IntRectangle::height);

    registration::class_<IntRectangleRange>("IntRectangleRange")
        .property("x_range", &IntRectangleRange::x_range)
        .property("y_range", &IntRectangleRange::y_range)
        .property("width_range", &IntRectangleRange::width_range)
        .property("height_range", &IntRectangleRange::height_range);

    registration::class_<Vector2D>("Vector2D")
        .property("x", &Vector2D::x)
        .property("y", &Vector2D::y)
        .property("space", &Vector2D::space);

    registration::class_<Vector1D>("Vector1D")
        .property("x", &Vector1D::x)
        .property("space", &Vector1D::space);

    registration::class_<PTZSpeed>("PTZSpeed")
        .property("pan_tilt", &PTZSpeed::pan_tilt)
        .property("zoom", &PTZSpeed::zoom);

    registration::class_<PTZVector>("PTZVector")
        .property("pan_tilt", &PTZVector::pan_tilt)
        .property("zoom", &PTZVector::zoom);

    registration::class_<PanTiltLimits>("PanTiltLimits")
        .property("uri", &PanTiltLimits::uri)
        .property("x_range", &PanTiltLimits::x_range)
        .property("y_range", &PanTiltLimits::y_range);

    registration::class_<ZoomLimits>("ZoomLimits")
        .property("address", &ZoomLimits::uri)
        .property("analytics_module_support", &ZoomLimits::x_range);

    registration::class_<PTZConfiguration>("PTZConfiguration")
        .property("node_token", &PTZConfiguration::node_token)
        .property("default_absolute_pant_tilt_position_space", &PTZConfiguration::default_absolute_pant_tilt_position_space)
        .property("default_absolute_zoom_position_space", &PTZConfiguration::default_absolute_zoom_position_space)
        .property("default_relative_pan_tilt_translation_space", &PTZConfiguration::default_relative_pan_tilt_translation_space)
        .property("default_relative_zoom_translation_space", &PTZConfiguration::default_relative_zoom_translation_space)
        .property("default_continuous_pan_tilt_velocity_space", &PTZConfiguration::default_continuous_pan_tilt_velocity_space)
        .property("default_continuous_zoom_velocity_space", &PTZConfiguration::default_continuous_zoom_velocity_space)
        .property("default_ptz_speed", &PTZConfiguration::default_ptz_speed)
        .property("default_ptz_timeout", &PTZConfiguration::default_ptz_timeout)
        .property("pan_tilt_limits", &PTZConfiguration::pan_tilt_limits)
        .property("zoom_limits", &PTZConfiguration::zoom_limits);

    registration::class_<VideoSourceConfiguration>("VideoSourceConfiguration")
        .property("source_token", &VideoSourceConfiguration::source_token)
        .property("bounds", &VideoSourceConfiguration::bounds);

    registration::class_<AudioSourceConfiguration>("AudioSourceConfiguration")
        .property("source_token", &AudioSourceConfiguration::source_token);

    registration::class_<VideoResolution>("VideoResolution")
        .property("width", &VideoResolution::width)
        .property("height", &VideoResolution::height);

    registration::class_<VideoRateControl>("VideoRateControl")
        .property("framerate_limit", &VideoRateControl::framerate_limit)
        .property("encoding_interval", &VideoRateControl::encoding_interval)
        .property("bitrate_limit", &VideoRateControl::bitrate_limit);

    registration::class_<Mpeg4Configuration>("Mpeg4Configuration")
        .property("gov_length", &Mpeg4Configuration::gov_length)
        .property("mpeg4_profile", &Mpeg4Configuration::mpeg4_profile);

    registration::class_<H264Configuration>("H264Configuration")
        .property("gov_length", &H264Configuration::gov_length)
        .property("mpeg4_profile", &H264Configuration::mpeg4_profile);

    registration::class_<MulticastConfiguration>("MulticastConfiguration")
        .property("ipv4_address", &MulticastConfiguration::ipv4_address)
        .property("port", &MulticastConfiguration::port)
        .property("ttl", &MulticastConfiguration::ttl)
        .property("auto_start", &MulticastConfiguration::auto_start);

    registration::class_<VideoEncoderConfiguration>("VideoEncoderConfiguration")
        .property("video_encoding", &VideoEncoderConfiguration::video_encoding)
        .property("video_resolution", &VideoEncoderConfiguration::video_resolution)
        .property("quality", &VideoEncoderConfiguration::quality)
        .property("video_rate_control", &VideoEncoderConfiguration::video_rate_control)
        .property("mpeg4_configuration", &VideoEncoderConfiguration::mpeg4_configuration)
        .property("h264_configuration", &VideoEncoderConfiguration::h264_configuration)
        .property("multicast_configuration", &VideoEncoderConfiguration::multicast_configuration)
        .property("session_timeout", &VideoEncoderConfiguration::session_timeout);

    registration::class_<Profile>("Profile")
        .property("name", &Profile::name)
        .property("token", &Profile::token)
        .property("fixed", &Profile::fixed)
        .property("video_source_configuration", &Profile::video_source_configuration)
        .property("audio_source_configuration", &Profile::audio_source_configuration)
        .property("video_encoder_configuration", &Profile::video_encoder_configuration)
        .property("ptz_configuration", &Profile::ptz_configuration);

    registration::class_<VideoSource>("VideoSource")
        .property("frame_rate", &VideoSource::frame_rate)
        .property("resolution", &VideoSource::resolution)
        .property("imaging", &VideoSource::imaging);

    registration::class_<VideoSourceConfigurationOptions>("VideoSourceConfigurationOptions")
        .property("bounds_range", &VideoSourceConfigurationOptions::bounds_range)
        .property("video_source_tokens_available", &VideoSourceConfigurationOptions::video_source_tokens_available);

    registration::class_<MediaUri>("MediaUri")
        .property("uri", &MediaUri::uri)
        .property("invalid_after_connect", &MediaUri::invalid_after_connect)
        .property("invalid_after_reboot", &MediaUri::invalid_after_reboot)
        .property("timeout", &MediaUri::timeout);

    registration::class_<StreamingCapabilities>("StreamingCapabilities")
        .property("rtp_multicast", &StreamingCapabilities::rtp_multicast)
        .property("rtp_uscoretcp", &StreamingCapabilities::rtp_uscoretcp)
        .property("rtp_uscorertsp_uscoretcp", &StreamingCapabilities::rtp_uscorertsp_uscoretcp)
        .property("non_aggregate_control", &StreamingCapabilities::non_aggregate_control)
        .property("no_rtsp_streaming", &StreamingCapabilities::no_rtsp_streaming);

    registration::class_<Capabilities>("Capabilities")
        .property("maximum_number_of_profiles", &Capabilities::maximum_number_of_profiles)
        .property("streaming_capabilities", &Capabilities::streaming_capabilities)
        .property("snapshot_uri", &Capabilities::snapshot_uri)
        .property("rotation", &Capabilities::rotation)
        .property("video_source_mode", &Capabilities::video_source_mode)
        .property("osd", &Capabilities::osd)
        .property("exi_compression", &Capabilities::exi_compression);

    registration::class_<Space2DDescriptionRange>("Space2DDescriptionRange")
        .property("uri", &Space2DDescriptionRange::uri)
        .property("x_range", &Space2DDescriptionRange::x_range)
        .property("y_range", &Space2DDescriptionRange::y_range);

    registration::class_<Space1DDescriptionRange>("Space1DDescriptionRange")
        .property("uri", &Space1DDescriptionRange::uri)
        .property("x_range", &Space1DDescriptionRange::x_range);

    registration::class_<PTZSpaces>("PTZSpaces")
        .property("absolute_pan_tilt_position_space", &PTZSpaces::absolute_pan_tilt_position_space)
        .property("absolute_zoom_position_space", &PTZSpaces::absolute_zoom_position_space)
        .property("relative_pantilt_translation_space", &PTZSpaces::relative_pantilt_translation_space)
        .property("relative_zoom_translation_space", &PTZSpaces::relative_zoom_translation_space)
        .property("continuous_pantilt_velocity_space", &PTZSpaces::continuous_pantilt_velocity_space)
        .property("continuous_zoom_velocity_space", &PTZSpaces::continuous_zoom_velocity_space)
        .property("pantilt_speed_space", &PTZSpaces::pantilt_speed_space)
        .property("zoom_speed_space", &PTZSpaces::zoom_speed_space);

    registration::class_<PTZConfigurationOptions>("PTZConfigurationOptions")
        .property("spaces", &PTZConfigurationOptions::spaces)
        .property("ptz_timeout", &PTZConfigurationOptions::ptz_timeout);

    registration::class_<Preset>("Preset")
        .property("name", &Preset::name)
        .property("token", &Preset::token)
        .property("ptz_position", &Preset::ptz_position);

    registration::class_<PTZMoveStatus>("PTZMoveStatus")
        .property("pan_tilt", &PTZMoveStatus::pan_tilt)
        .property("zoom", &PTZMoveStatus::zoom);

    registration::class_<PTZStatus>("PTZStatus")
        .property("position", &PTZStatus::position)
        .property("move_status", &PTZStatus::move_status)
        .property("error", &PTZStatus::error)
        .property("utc_time", &PTZStatus::utc_time);

    registration::class_<PTZCapabilitiesInformation>("PTZCapabilitiesInformation")
        .property("e_flip", &PTZCapabilitiesInformation::e_flip)
        .property("reverse", &PTZCapabilitiesInformation::reverse)
        .property("get_compatible_configurations", &PTZCapabilitiesInformation::get_compatible_configurations)
        .property("move_status", &PTZCapabilitiesInformation::move_status)
        .property("status_position", &PTZCapabilitiesInformation::status_position);

    registration::class_<PTZNode>("PTZNode")
        .property("name", &PTZNode::name)
        .property("supported_ptz_spaces", &PTZNode::supported_ptz_spaces)
        .property("maximum_number_of_presets", &PTZNode::maximum_number_of_presets)
        .property("home_supported", &PTZNode::home_supported)
        .property("auxiliary_commands", &PTZNode::auxiliary_commands)
        .property("fixed_home_position", &PTZNode::fixed_home_position);

    registration::class_<FRange>("FRange")
        .property("min", &FRange::first)
        .property("max", &FRange::second);

    registration::class_<IRange>("IRange")
        .property("min", &IRange::first)
        .property("max", &IRange::second);

    registration::class_<DurationRange>("DurationRange")
        .property("min", &DurationRange::first)
        .property("max", &DurationRange::second);
}
