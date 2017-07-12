//
// Created by svakhreev on 30.03.17.
//

#ifndef CAMERAMANAGERCORE_PARCEFROMURL_HPP
#define CAMERAMANAGERCORE_PARCEFROMURL_HPP

#include <unordered_map>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <vector>
#include <string_view>
#include <range/v3/all.hpp>

namespace cameramanagercore::schemas
{

std::unordered_map<std::string, std::string> ParseFromUrl(std::string str);

}


#endif //CAMERAMANAGERCORE_PARCEFROMURL_HPP
