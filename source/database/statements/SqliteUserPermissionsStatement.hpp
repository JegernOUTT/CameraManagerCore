//
// Created by svakhreev on 20.02.17.
//

#ifndef CAMERAMANAGERCORE_SQLITEUSERPERMISSIONSSTATEMENT_HPP
#define CAMERAMANAGERCORE_SQLITEUSERPERMISSIONSSTATEMENT_HPP

#include <vector>
#include <memory>
#include <initializer_list>
#include <tuple>

#include <boost/format.hpp>

#include "../statements/Statements.hpp"
#include "../adapters/SqlLanguageAdapter.hpp"
#include "../../model/UserPermissions.hpp"

namespace cameramanagercore::database::statements
{

using namespace adapters;
using namespace cameramanagercore::model;

using UserPermissionsPtr = std::shared_ptr<UserPermissions>;

template<typename AdapterPtr>
struct CreateTableImpl<AdapterPtr, UserPermissionsPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Creating user_permissions table";

        return ErrorHandler
        {
            [&]
            {
                (*adapter) << "CREATE TABLE IF NOT EXISTS user_permissions (\n"
                              "    user_id       INTEGER REFERENCES user (id) ON DELETE CASCADE\n"
                              "                                               ON UPDATE CASCADE\n"
                              "                          NOT NULL,\n"
                              "    permission_id INTEGER REFERENCES permission (id) ON DELETE CASCADE\n"
                              "                                                     ON UPDATE CASCADE\n"
                              "                          NOT NULL\n"
                              ");"
                           >> ExecuteEmpty{};;
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct GetImpl<AdapterPtr, UserPermissionsPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Get user_permissions from DB";

        return ErrorHandlerWithData<std::vector<UserPermissionsPtr>>
        {
            [&]
            {
                std::vector<UserPermissionsPtr> user_permissions;
                auto visitor = [&](int user_id,
                                   int permission_id)
                { user_permissions.emplace_back(std::make_shared<UserPermissions>(UserPermissions{ user_id,
                                                                                                   permission_id })); };
                (*adapter) << (SelectFrom | UserPermissionsTable)
                           >> visitor;

                return user_permissions;
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct AddImpl<AdapterPtr, UserPermissionsPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<UserPermissionsPtr> user_permissions)
    {
        LOG(info) << "Add new user_permissions to DB";
        std::for_each(user_permissions.begin(), user_permissions.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& us_perm: user_permissions)
                {
                    (*adapter) << (InsertTo | UserPermissionsTable | Values(*us_perm))
                               >> ExecuteEmpty{};
                }
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct ChangeImpl<AdapterPtr, UserPermissionsPtr>
{
    SqliteStatus operator()(AdapterPtr adapter, std::initializer_list<UserPermissionsPtr> user_permissions)
    {
        return {};
    }
};

template<typename AdapterPtr>
struct RemoveImpl<AdapterPtr, UserPermissionsPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<UserPermissionsPtr> user_permissions)
    {
        LOG(info) << "Remove user_permissions from DB";
        std::for_each(user_permissions.begin(), user_permissions.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& us_perm: user_permissions)
                {
                    (*adapter) << (DeleteFrom | UserPermissionsTable | WhereRowLike(*us_perm))
                               >> ExecuteEmpty{};
                }
            }
        }.result();
    }
};

}

#endif //CAMERAMANAGERCORE_SQLITEUSERPERMISSIONSSTATEMENT_HPP
