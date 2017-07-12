//
// Created by svakhreev on 29.12.16.
//

#ifndef CAMERAMANAGERCORE_SQLITECAMERASTATEMENT_HPP
#define CAMERAMANAGERCORE_SQLITECAMERASTATEMENT_HPP

#include <vector>
#include <memory>
#include <initializer_list>
#include <tuple>

#include <boost/format.hpp>

#include "../statements/Statements.hpp"
#include "../adapters/SqlLanguageAdapter.hpp"
#include "../../model/Camera.hpp"

namespace cameramanagercore::database::statements
{

using namespace adapters;
using namespace cameramanagercore::model;

using CameraPtr = std::shared_ptr<Camera>;

template<typename AdapterPtr>
struct CreateTableImpl<AdapterPtr, CameraPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Creating camera table";

        return ErrorHandler
        {
            [&]
            {
                (*adapter) << "CREATE TABLE IF NOT EXISTS camera (\n"
                              "    id               INTEGER PRIMARY KEY AUTOINCREMENT\n"
                              "                             NOT NULL,\n"
                              "    name             TEXT    NOT NULL,\n"
                              "    url              TEXT    NOT NULL,\n"
                              "    login            TEXT    NOT NULL,\n"
                              "    password         TEXT    NOT NULL,\n"
                              "    interaction_id   INTEGER NOT NULL\n"
                              "                             REFERENCES interaction (id) ON DELETE CASCADE\n"
                              "                                                         ON UPDATE CASCADE,\n"
                              "    idle_timeout_sec INT,\n"
                              "    on_idle_action   TEXT\n"
                              ");"
                    >> ExecuteEmpty{};
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct GetImpl<AdapterPtr, CameraPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Get cameras from DB";

        return ErrorHandlerWithData<std::vector<CameraPtr>>
        {
            [&]
            {
                std::vector<CameraPtr> cameras;
                auto visitor = [&](int id,
                                   std::string name,
                                   std::string url,
                                   std::string login,
                                   std::string password,
                                   int interaction,
                                   int idle_timeout_sec,
                                   std::string on_idle_action)
                { cameras.emplace_back(std::make_shared<Camera>(Camera{ name, url,
                                                                        login, password,
                                                                        static_cast<CameraInteraction>(interaction),
                                                                        idle_timeout_sec,
                                                                        on_idle_action, id })); };
                (*adapter) << (SelectFrom | CameraTable)
                           >> visitor;

                return cameras;
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct AddImpl<AdapterPtr, CameraPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<CameraPtr> cameras)
    {
        LOG(info) << "Add new cameras to DB";
        std::for_each(cameras.begin(), cameras.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& camera: cameras)
                {
                    (*adapter) << (InsertTo | CameraTable | Values(*camera))
                               >> ExecuteEmpty{}
                               << (SelectFrom | CameraTable | WhereRowLike(*camera))
                               >> std::tie(camera->id);
                }
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct ChangeImpl<AdapterPtr, CameraPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<CameraPtr> cameras)
    {
        LOG(info) << "Update cameras from DB";
        std::for_each(cameras.begin(), cameras.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& camera: cameras)
                {
                    (*adapter) << (Update | CameraTable | From(*camera)
                                                        | Where((boost::format("id = %1%") % camera->id).str()))
                               >> ExecuteEmpty{};
                };
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct RemoveImpl<AdapterPtr, CameraPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<CameraPtr> cameras)
    {
        LOG(info) << "Remove cameras from DB";
        std::for_each(cameras.begin(), cameras.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& camera: cameras)
                {
                    (*adapter) << (DeleteFrom | CameraTable
                                              | Where((boost::format("id = %1%") % camera->id).str()))
                               >> ExecuteEmpty{};
                }
            }
        }.result();
    }
};

}

#endif //CAMERAMANAGERCORE_SQLITECAMERASTATEMENT_HPP
