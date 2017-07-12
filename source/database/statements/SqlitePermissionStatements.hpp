//
// Created by svakhreev on 20.02.17.
//

#ifndef CAMERAMANAGERCORE_SQLITEPERMISSIONSTATEMENTS_HPP
#define CAMERAMANAGERCORE_SQLITEPERMISSIONSTATEMENTS_HPP

#include <vector>
#include <memory>
#include <initializer_list>
#include <tuple>

#include <boost/format.hpp>

#include "../statements/Statements.hpp"
#include "../adapters/SqlLanguageAdapter.hpp"
#include "../../model/Permission.hpp"

namespace cameramanagercore::database::statements
{

using namespace adapters;
using namespace cameramanagercore::model;

using PermissionPtr = std::shared_ptr<Permission>;

template<typename AdapterPtr>
struct CreateTableImpl<AdapterPtr, PermissionPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Creating permissions table";

        return ErrorHandler
        {
            [&]
            {
                (*adapter) << "CREATE TABLE IF NOT EXISTS permission (\n"
                              "    id        INTEGER PRIMARY KEY AUTOINCREMENT\n"
                              "                      NOT NULL,\n"
                              "    name      TEXT    NOT NULL,\n"
                              "    regex_url TEXT    NOT NULL\n"
                              ");"
                           >> ExecuteEmpty{};;
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct GetImpl<AdapterPtr, PermissionPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Get permissions from DB";

        return ErrorHandlerWithData<std::vector<PermissionPtr>>
        {
            [&]
            {
                std::vector<PermissionPtr> permissions;
                auto visitor = [&](int id,
                                   std::string name,
                                   std::string regex_url)
                { permissions.emplace_back(std::make_shared<Permission>(Permission{ name,
                                                                                    regex_url,
                                                                                    id })); };
                (*adapter) << (SelectFrom | PermissionTable)
                           >> visitor;

                return permissions;
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct AddImpl<AdapterPtr, PermissionPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<PermissionPtr> permissions)
    {
        LOG(info) << "Add new permissions to DB";
        std::for_each(permissions.begin(), permissions.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& permission: permissions)
                {
                    (*adapter) << (InsertTo | PermissionTable | Values(*permission))
                               >> ExecuteEmpty{}
                               << (SelectFrom | PermissionTable | WhereRowLike(*permission))
                               >> std::tie(permission->id);
                }
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct ChangeImpl<AdapterPtr, PermissionPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<PermissionPtr> permissions)
    {
        LOG(info) << "Update permissions from DB";
        std::for_each(permissions.begin(), permissions.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& permission: permissions)
                {
                    (*adapter) << (Update | PermissionTable | From(*permission)
                                                            | Where((boost::format("id = %1%") % permission->id).str()))
                               >> ExecuteEmpty{};
                };
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct RemoveImpl<AdapterPtr, PermissionPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<PermissionPtr> permissions)
    {
        LOG(info) << "Remove permissions from DB";
        std::for_each(permissions.begin(), permissions.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& permission: permissions)
                {
                    (*adapter) << (DeleteFrom | PermissionTable
                                              | Where((boost::format("id = %1%") % permission->id).str()))
                               >> ExecuteEmpty{};
                }
            }
        }.result();
    }
};

}

#endif //CAMERAMANAGERCORE_SQLITEPERMISSIONSTATEMENTS_HPP
