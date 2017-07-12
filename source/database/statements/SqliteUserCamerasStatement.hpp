//
// Created by svakhreev on 20.02.17.
//

#ifndef CAMERAMANAGERCORE_SQLITEUSERCAMERASSTATEMENT_HPP
#define CAMERAMANAGERCORE_SQLITEUSERCAMERASSTATEMENT_HPP

#include <vector>
#include <memory>
#include <initializer_list>
#include <tuple>

#include <boost/format.hpp>

#include "../statements/Statements.hpp"
#include "../adapters/SqlLanguageAdapter.hpp"
#include "../../model/UserCameras.hpp"

namespace cameramanagercore::database::statements
{

using namespace adapters;
using namespace cameramanagercore::model;

using UserCamerasPtr = std::shared_ptr<UserCameras>;

template<typename AdapterPtr>
struct CreateTableImpl<AdapterPtr, UserCamerasPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Creating user_cameras table";

        return ErrorHandler
        {
            [&]
            {
                (*adapter) << "CREATE TABLE IF NOT EXISTS user_cameras (\n"
                              "    user_id   INTEGER NOT NULL\n"
                              "                      REFERENCES user (id) ON DELETE CASCADE\n"
                              "                                           ON UPDATE CASCADE,\n"
                              "    camera_id INTEGER NOT NULL\n"
                              "                      REFERENCES camera (id) ON DELETE CASCADE\n"
                              "                                             ON UPDATE CASCADE\n"
                              ");"
                           >> ExecuteEmpty{};;
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct GetImpl<AdapterPtr, UserCamerasPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Get user_cameras from DB";

        return ErrorHandlerWithData<std::vector<UserCamerasPtr>>
        {
            [&]
            {
                std::vector<UserCamerasPtr> user_cameras;
                auto visitor = [&](int user_id,
                                   int camera_id)
                { user_cameras.emplace_back(std::make_shared<UserCameras>(UserCameras{ user_id,
                                                                                       camera_id })); };
                (*adapter) << (SelectFrom | UserCamerasTable)
                           >> visitor;

                return user_cameras;
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct AddImpl<AdapterPtr, UserCamerasPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<UserCamerasPtr> user_cameras)
    {
        LOG(info) << "Add new user_cameras to DB";
        std::for_each(user_cameras.begin(), user_cameras.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& us_cam: user_cameras)
                {
                    (*adapter) << (InsertTo | UserCamerasTable | Values(*us_cam))
                               >> ExecuteEmpty{};
                }
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct ChangeImpl<AdapterPtr, UserCamerasPtr>
{
    SqliteStatus operator()(AdapterPtr adapter, std::initializer_list<UserCamerasPtr> user_cameras)
    {
        return {};
    }
};

template<typename AdapterPtr>
struct RemoveImpl<AdapterPtr, UserCamerasPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<UserCamerasPtr> user_cameras)
    {
        LOG(info) << "Remove user_cameras from DB";
        std::for_each(user_cameras.begin(), user_cameras.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& us_cam: user_cameras)
                {
                    (*adapter) << (DeleteFrom | UserCamerasTable | WhereRowLike(*us_cam))
                               >> ExecuteEmpty{};
                }
            }
        }.result();
    }
};

}

#endif //CAMERAMANAGERCORE_SQLITEUSERCAMERASSTATEMENT_HPP
