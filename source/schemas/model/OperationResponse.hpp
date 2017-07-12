//
// Created by svakhreev on 27.03.17.
//

#ifndef CAMERAMANAGERCORE_OPERATIONSTATUS_HPP
#define CAMERAMANAGERCORE_OPERATIONSTATUS_HPP

#include <string>
#include "Parcers.hpp"

namespace cameramanagercore::schemas::model
{

struct OperationResponse
{
    int code;
    std::string message;
};

}


#endif //CAMERAMANAGERCORE_OPERATIONSTATUS_HPP
