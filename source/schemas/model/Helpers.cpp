//
// Created by svakhreev on 04.04.17.
//

#include "Helpers.hpp"
#include <rttr/registration>

using namespace rttr;

namespace cameramanagercore::schemas::model
{

RTTR_REGISTRATION
{
    registration::class_<StreamUriHelper>("StreamUriHelper")
        .constructor<>()
        .property("stream_type",        &StreamUriHelper::stream_type)
        .property("transport_protocol", &StreamUriHelper::transport_protocol);
}

}