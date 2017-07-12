//
// Created by svakhreev on 04.04.17.
//

#ifndef CAMERAMANAGERCORE_HELPERS_HPP
#define CAMERAMANAGERCORE_HELPERS_HPP

#include <onvif_wrapper/OnvifPod.hpp>

namespace cameramanagercore::schemas::model
{

using namespace onvifwrapper;

struct StreamUriHelper
{
    StreamType stream_type;
    TransportProtocol transport_protocol;
};

}

#endif //CAMERAMANAGERCORE_HELPERS_HPP
