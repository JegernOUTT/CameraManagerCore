//
// Created by svakhreev on 07.02.17.
//

#ifndef CAMERAMANAGERCORE_PERMISSION_HPP
#define CAMERAMANAGERCORE_PERMISSION_HPP

#include <string>
#include <ostream>

#include "Common.hpp"

namespace cameramanagercore::model
{

struct Permission
{
    std::string name;
    std::string regex_url;
    int id;
};

}


#endif //CAMERAMANAGERCORE_PERMISSION_HPP
