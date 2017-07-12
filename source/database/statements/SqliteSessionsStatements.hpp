//
// Created by svakhreev on 29.03.17.
//

#ifndef CAMERAMANAGERCORE_SQLITESESSIONSSTATEMENTS_HPP
#define CAMERAMANAGERCORE_SQLITESESSIONSSTATEMENTS_HPP

#include <vector>
#include <memory>
#include <initializer_list>
#include <tuple>

#include <boost/format.hpp>

#include "../statements/Statements.hpp"
#include "../adapters/SqlLanguageAdapter.hpp"
#include "../../model/Sessions.hpp"

namespace cameramanagercore::database::statements
{

using namespace adapters;
using namespace cameramanagercore::model;

using SessionsPtr = std::shared_ptr<Sessions>;

template<typename AdapterPtr>
struct CreateTableImpl<AdapterPtr, SessionsPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Creating session table";

        return ErrorHandler
            {
                [&]
                {
                    (*adapter) << "CREATE TABLE IF NOT EXISTS sessions (\n"
                                  "    session_hash     STRING   PRIMARY KEY,\n"
                                  "    user_id          INTEGER  REFERENCES user (id) ON UPDATE CASCADE,\n"
                                  "    creation_time    DATETIME,\n"
                                  "    destruction_time DATETIME\n"
                                  ");"
                               >> ExecuteEmpty{};
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct GetImpl<AdapterPtr, SessionsPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Get sessions from DB";

        return ErrorHandlerWithData<std::vector<SessionsPtr>>
            {
                [&]
                {
                    std::vector<SessionsPtr> sessions;
                    auto visitor = [&](std::string session_hash,
                                       int user_id,
                                       sqlite3_int64 creation_time,
                                       int destruction_time)
                    { sessions.emplace_back(std::make_shared<Sessions>(Sessions{ session_hash, user_id,
                                                                                 creation_time, destruction_time })); };
                    (*adapter) << (SelectFrom | SessionsTable)
                               >> visitor;

                    return sessions;
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct AddImpl<AdapterPtr, SessionsPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<SessionsPtr> sessions)
    {
        LOG(info) << "Add new sessions to DB";
        std::for_each(sessions.begin(), sessions.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
            {
                [&]
                {
                    for (const auto& session: sessions)
                    {
                        (*adapter) << (InsertTo | SessionsTable | Values(*session))
                                   >> ExecuteEmpty{};
                    }
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct ChangeImpl<AdapterPtr, SessionsPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<SessionsPtr> sessions)
    {
        LOG(info) << "Update sessions from DB";
        std::for_each(sessions.begin(), sessions.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
            {
                [&]
                {
                    for (const auto& session: sessions)
                    {
                        (*adapter) << (Update | SessionsTable | From(*session)
                                              | Where((boost::format("session_hash = '%1%'")
                                                       % session->session_hash).str()))
                                   >> ExecuteEmpty{};
                    };
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct RemoveImpl<AdapterPtr, SessionsPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<SessionsPtr> sessions)
    {
        LOG(info) << "Remove sessions from DB";
        std::for_each(sessions.begin(), sessions.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
            {
                [&]
                {
                    for (const auto& session: sessions)
                    {
                        (*adapter) << (DeleteFrom | SessionsTable
                                                  | Where((boost::format("session_hash = '%1%'")
                                                           % session->session_hash).str()))
                                   >> ExecuteEmpty{};
                    }
                }
            }.result();
    }
};

}

#endif //CAMERAMANAGERCORE_SQLITESESSIONSSTATEMENTS_HPP
