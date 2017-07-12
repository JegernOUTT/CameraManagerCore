//
// Created by svakhreev on 22.03.17.
//

#ifndef CAMERAMANAGERCORE_SQLITEUTILITY_HPP
#define CAMERAMANAGERCORE_SQLITEUTILITY_HPP

#include <string_view>
#include <sqlite_modern_cpp.h>

namespace cameramanagercore::database::utility
{

bool Exist(std::string_view db_name);

}

#endif //CAMERAMANAGERCORE_SQLITEUTILITY_HPP
