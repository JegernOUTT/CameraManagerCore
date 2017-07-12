//
// Created by svakhreev on 22.03.17.
//

#include "SqliteUtility.hpp"

namespace cameramanagercore::database::utility
{

bool Exist(std::string_view db_name)
{
    try
    {
        sqlite::sqlite_config c { sqlite::OpenFlags::READONLY };
        sqlite::database db(db_name.data(), c);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

}