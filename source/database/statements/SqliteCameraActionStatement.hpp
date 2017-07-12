//
// Created by svakhreev on 29.03.17.
//

#ifndef CAMERAMANAGERCORE_SQLITECAMERAACTIONSTATEMENT_HPP
#define CAMERAMANAGERCORE_SQLITECAMERAACTIONSTATEMENT_HPP

#include <vector>
#include <memory>
#include <initializer_list>
#include <tuple>

#include <boost/format.hpp>

#include "../statements/Statements.hpp"
#include "../adapters/SqlLanguageAdapter.hpp"
#include "../../model/CameraInformation.hpp"

namespace cameramanagercore::database::statements
{

using namespace adapters;
using namespace cameramanagercore::model;

using CameraActionPtr = std::shared_ptr<CameraAction>;

template<typename AdapterPtr>
struct CreateTableImpl<AdapterPtr, CameraActionPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Creating camera_action table";

        return ErrorHandler
            {
                [&]
                {
                    (*adapter) << "CREATE TABLE IF NOT EXISTS camera_action (\n"
                                  "    id           INTEGER  PRIMARY KEY AUTOINCREMENT,\n"
                                  "    succeed      BOOLEAN,\n"
                                  "    camera_id    INTEGER  REFERENCES camera (id) ON UPDATE CASCADE,\n"
                                  "    session_hash TEXT     REFERENCES sessions (session_hash),\n"
                                  "    time         DATETIME,\n"
                                  "    [action]     TEXT\n"
                                  ");"
                               >> ExecuteEmpty{};
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct GetImpl<AdapterPtr, CameraActionPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Get camera_actions from DB";

        return ErrorHandlerWithData<std::vector<CameraActionPtr>>
            {
                [&]
                {
                    std::vector<CameraActionPtr> camera_actions;
                    auto visitor = [&](int id,
                                       int succeed,
                                       int camera_id,
                                       std::string session_hash,
                                       sqlite3_int64 time,
                                       std::string action)
                    { camera_actions.emplace_back(
                        std::make_shared<CameraAction>(CameraAction{ static_cast<bool>(succeed),
                                                                     camera_id,
                                                                     session_hash,
                                                                     time,
                                                                     action,
                                                                     id })); };
                    (*adapter) << (SelectFrom | CameraActionTable)
                               >> visitor;

                    return camera_actions;
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct AddImpl<AdapterPtr, CameraActionPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<CameraActionPtr> camera_actions)
    {
        LOG(info) << "Add new camera_actions to DB";
        std::for_each(camera_actions.begin(), camera_actions.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
            {
                [&]
                {
                    for (const auto& camera_action: camera_actions)
                    {
                        (*adapter) << (InsertTo | CameraActionTable | Values(*camera_action))
                                   >> ExecuteEmpty{}
                                   << (SelectFrom | CameraActionTable | WhereRowLike(*camera_action))
                                   >> std::tie(camera_action->id);
                    }
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct ChangeImpl<AdapterPtr, CameraActionPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<CameraActionPtr> camera_actions)
    {
        LOG(info) << "Update camera_actions from DB";
        std::for_each(camera_actions.begin(), camera_actions.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
            {
                [&]
                {
                    for (const auto& camera_action: camera_actions)
                    {
                        (*adapter) << (Update | CameraActionTable | From(*camera_action)
                                              | Where((boost::format("id = %1%") % camera_action->id).str()))
                                   >> ExecuteEmpty{};
                    };
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct RemoveImpl<AdapterPtr, CameraActionPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<CameraActionPtr> camera_actions)
    {
        LOG(info) << "Remove camera_actions from DB";
        std::for_each(camera_actions.begin(), camera_actions.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
            {
                [&]
                {
                    for (const auto& camera_action: camera_actions)
                    {
                        (*adapter) << (DeleteFrom | CameraActionTable
                                                  | Where((boost::format("id = %1%") % camera_action->id).str()))
                                   >> ExecuteEmpty{};
                    }
                }
            }.result();
    }
};

}

#endif //CAMERAMANAGERCORE_SQLITECAMERAACTIONSTATEMENT_HPP
