//EXIST
// Created by svakhreev on 29.12.16.
//

#ifndef CAMERAMANAGERCORE_USERSERIALIZER_HPP
#define CAMERAMANAGERCORE_USERSERIALIZER_HPP

#include <vector>
#include <memory>
#include <initializer_list>
#include <tuple>

#include <boost/format.hpp>

#include "../statements/Statements.hpp"
#include "../adapters/SqlLanguageAdapter.hpp"
#include "../../model/User.hpp"

namespace cameramanagercore::database::statements
{

using namespace adapters;
using namespace cameramanagercore::model;

using UserPtr = std::shared_ptr<User>;

template<typename AdapterPtr>
struct CreateTableImpl<AdapterPtr, UserPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Creating users table";

        return ErrorHandler
        {
            [&]
            {
                (*adapter) << "CREATE TABLE IF NOT EXISTS user (\n"
                              "    id    INTEGER PRIMARY KEY AUTOINCREMENT\n"
                              "                     NOT NULL,\n"
                              "    name     TEXT    NOT NULL,\n"
                              "    password TEXT    NOT NULL,\n"
                              "    event_categories TEXT\n"
                              ");"
                           >> ExecuteEmpty{};;
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct GetImpl<AdapterPtr, UserPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Get users from DB";

        return ErrorHandlerWithData<std::vector<UserPtr>>
        {
            [&]
            {
                std::vector<UserPtr> users;
                auto visitor = [&](int id,
                                   std::string name,
                                   std::string password,
                                   std::string event_categories)
                { users.emplace_back(std::make_shared<User>(User{ name, password, event_categories, id })); };
                (*adapter) << (SelectFrom | UserTable)
                           >> visitor;

                return users;
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct AddImpl<AdapterPtr, UserPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<UserPtr> users)
    {
        LOG(info) << "Add new users to DB";
        std::for_each(users.begin(), users.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& user: users)
                {
                    (*adapter) << (InsertTo | UserTable | Values(*user))
                               >> ExecuteEmpty{}
                               << (SelectFrom | UserTable | WhereRowLike(*user))
                               >> std::tie(user->id);
                }
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct ChangeImpl<AdapterPtr, UserPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<UserPtr> users)
    {
        LOG(info) << "Update users from DB";
        std::for_each(users.begin(), users.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& user: users)
                {
                    (*adapter) << (Update | UserTable | From(*user)
                                          | Where((boost::format("id = %1%") % user->id).str()))
                               >> ExecuteEmpty{};
                };
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct RemoveImpl<AdapterPtr, UserPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<UserPtr> users)
    {
        LOG(info) << "Remove users from DB";
        std::for_each(users.begin(), users.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& user: users)
                {
                    (*adapter) << (DeleteFrom | UserTable
                                              | Where((boost::format("id = %1%") % user->id).str()))
                               >> ExecuteEmpty{};
                }
            }
        }.result();
    }
};

}



#endif //CAMERAMANAGERCORE_USERSERIALIZER_HPP
