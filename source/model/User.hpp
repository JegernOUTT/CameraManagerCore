//
// Created by svakhreev on 29.12.16.
//

#ifndef CAMERAMANAGERCORE_USER_HPP
#define CAMERAMANAGERCORE_USER_HPP

#include <string>

#include "Common.hpp"

namespace cameramanagercore::model
{

struct User
{
    std::string name;
    std::string password;
    std::string event_categories;
    int id;
};

}


#endif //CAMERAMANAGERCORE_USER_HPP
