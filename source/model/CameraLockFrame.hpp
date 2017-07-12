//
// Created by svakhreev on 10.03.17.
//

#ifndef CAMERAMANAGERCORE_CAMERALOCKFRAME_HPP
#define CAMERAMANAGERCORE_CAMERALOCKFRAME_HPP

#include <string>

namespace cameramanagercore::model
{

struct CameraLockFrame
{
    bool locked;

    int camera_id;
    int user_id;
    int locked_time;
    int timeout;

    std::string lock_hash;
};

}

#endif //CAMERAMANAGERCORE_CAMERALOCKFRAME_HPP
