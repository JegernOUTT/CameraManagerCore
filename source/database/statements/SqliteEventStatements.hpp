//
// Created by svakhreev on 20.02.17.
//

#ifndef CAMERAMANAGERCORE_SQLITEEVENTSTATEMENTS_HPP
#define CAMERAMANAGERCORE_SQLITEEVENTSTATEMENTS_HPP

#include <vector>
#include <memory>
#include <initializer_list>
#include <tuple>

#include <boost/format.hpp>

#include "../statements/Statements.hpp"
#include "../adapters/SqlLanguageAdapter.hpp"
#include "../../model/Event.hpp"

namespace cameramanagercore::database::statements
{

using namespace adapters;
using namespace cameramanagercore::model;

using EventPtr = std::shared_ptr<Event>;

template<typename AdapterPtr>
struct CreateTableImpl<AdapterPtr, EventPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Creating event table";

        return ErrorHandler
            {
                [&]
                {
                    (*adapter) << "CREATE TABLE IF NOT EXISTS event (\n"
                                  "    hash                  TEXT     PRIMARY KEY\n"
                                  "                                   NOT NULL,\n"
                                  "    name                      TEXT,\n"
                                  "    description               TEXT,\n"
                                  "    camera_id                 INTEGER  REFERENCES camera (id) ON UPDATE CASCADE,\n"
                                  "    category                  TEXT,\n"
                                  "    created_session_hash      TEXT,\n"
                                  "    created_time              DATETIME,\n"
                                  "    on_create_action          TEXT,\n"
                                  "    processed                 BOOLEAN,\n"
                                  "    processed_time            DATETIME,\n"
                                  "    acknowledged              BOOLEAN,\n"
                                  "    acknowledge_session_hash  TEXT,\n"
                                  "    acknowledged_time         DATETIME,\n"
                                  "    on_acknowledge_action     TEXT,\n"
                                  "    has_video                 BOOLEAN\n"
                                  ");"
                               >> ExecuteEmpty{};
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct GetImpl<AdapterPtr, EventPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Get events from DB";

        return ErrorHandlerWithData<std::vector<EventPtr>>
        {
            [&]
            {
                std::vector<EventPtr> events;
                auto visitor = [&](std::string hash,
                                   std::string name,
                                   std::string description,
                                   int camera_id,
                                   std::string category,
                                   std::string created_session_hash,
                                   sqlite3_int64 created_time,
                                   std::string on_create_action,
                                   int processed,
                                   sqlite3_int64 processed_time,
                                   int acknowledged,
                                   std::string acknowledge_session_hash,
                                   sqlite3_int64 acknowledged_time,
                                   std::string on_acknowledge_action,
                                   int has_video)
                { events.emplace_back(std::make_shared<Event>(Event{ hash, name, description, camera_id, category,
                                                                     created_session_hash, created_time, on_create_action,
                                                                     static_cast<bool>(processed), processed_time,
                                                                     static_cast<bool>(acknowledged), acknowledge_session_hash,
                                                                     acknowledged_time, on_acknowledge_action,
                                                                     static_cast<bool>(has_video) })); };
                (*adapter) << (SelectFrom | EventTable)
                           >> visitor;

                return events;
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct AddImpl<AdapterPtr, EventPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<EventPtr> events)
    {
        LOG(info) << "Add new events to DB";
        std::for_each(events.begin(), events.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& event: events)
                {
                    (*adapter) << (InsertTo | EventTable | Values(*event))
                               >> ExecuteEmpty{};
                }
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct ChangeImpl<AdapterPtr, EventPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<EventPtr> events)
    {
        LOG(info) << "Update events from DB";
        std::for_each(events.begin(), events.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& event: events)
                {
                    (*adapter) << (Update | EventTable | From(*event)
                                          | Where((boost::format("hash = \"%1%\"") % event->hash).str()))
                               >> ExecuteEmpty{};
                };
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct RemoveImpl<AdapterPtr, EventPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<EventPtr> events)
    {
        LOG(info) << "Remove events from DB";
        std::for_each(events.begin(), events.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& event: events)
                {
                    (*adapter) << (DeleteFrom | EventTable
                                              | Where((boost::format("hash = \"%1%\"") % event->hash).str()))
                               >> ExecuteEmpty{};
                }
            }
        }.result();
    }
};

}

#endif //CAMERAMANAGERCORE_SQLITEEVENTSTATEMENTS_HPP
