//
// Created by svakhreev on 29.03.17.
//

#ifndef CAMERAMANAGERCORE_SESSIONS_HPP
#define CAMERAMANAGERCORE_SESSIONS_HPP

#include <string>

namespace cameramanagercore::model
{

using SqliteDateTime = int64_t;

struct Sessions
{
    std::string session_hash;
    int user_id;
    SqliteDateTime creation_time;
    SqliteDateTime destruction_time;
};

}

#endif //CAMERAMANAGERCORE_SESSIONS_HPP
