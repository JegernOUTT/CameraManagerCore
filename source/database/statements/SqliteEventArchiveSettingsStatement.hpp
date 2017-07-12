//
// Created by svakhreev on 06.04.17.
//

#ifndef CAMERAMANAGERCORE_SQLITEEVENTARCHIVESETTINGSSTATEMENT_HPP
#define CAMERAMANAGERCORE_SQLITEEVENTARCHIVESETTINGSSTATEMENT_HPP

#include <vector>
#include <memory>
#include <initializer_list>
#include <tuple>

#include <boost/format.hpp>

#include "../statements/Statements.hpp"
#include "../adapters/SqlLanguageAdapter.hpp"
#include "../../model/EventArchiveSettings.hpp"

namespace cameramanagercore::database::statements
{

using namespace adapters;
using namespace cameramanagercore::model;

using EventArchiveSettingsPtr = std::shared_ptr<EventArchiveSettings>;

template<typename AdapterPtr>
struct CreateTableImpl<AdapterPtr, EventArchiveSettingsPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Creating event_archive_set table";

        return ErrorHandler
            {
                [&]
                {
                    (*adapter) << "CREATE TABLE IF NOT EXISTS event_archive_settings (\n"
                                  "    id               INTEGER PRIMARY KEY,\n"
                                  "    camera_id        INTEGER REFERENCES camera (id) ON UPDATE CASCADE,\n"
                                  "    event_categories STRING,\n"
                                  "    buffer_seconds   INTEGER,\n"
                                  "    record_seconds   INTEGER\n"
                                  ");"
                               >> ExecuteEmpty{};
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct GetImpl<AdapterPtr, EventArchiveSettingsPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Get event_archive_settings from DB";

        return ErrorHandlerWithData<std::vector<EventArchiveSettingsPtr>>
            {
                [&]
                {
                    std::vector<EventArchiveSettingsPtr> event_archive_settings;
                    auto visitor = [&](int id,
                                       int camera_id,
                                       std::string event_categories,
                                       int buffer_seconds,
                                       int record_seconds)
                    { event_archive_settings.emplace_back(
                        std::make_shared<EventArchiveSettings>(EventArchiveSettings{ camera_id,
                                                                                     event_categories,
                                                                                     buffer_seconds,
                                                                                     record_seconds,
                                                                                     id })); };
                    (*adapter) << (SelectFrom | EventArchiveSettingsTable)
                               >> visitor;
                    return event_archive_settings;
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct AddImpl<AdapterPtr, EventArchiveSettingsPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<EventArchiveSettingsPtr> event_archive_settings)
    {
        LOG(info) << "Add new event_archive_settings to DB";
        std::for_each(event_archive_settings.begin(),
                      event_archive_settings.end(),
                      [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
            {
                [&]
                {
                    for (const auto& event_archive_set: event_archive_settings)
                    {
                        (*adapter) << (InsertTo | EventArchiveSettingsTable | Values(*event_archive_set))
                                   >> ExecuteEmpty{}
                                   << (SelectFrom | EventArchiveSettingsTable | WhereRowLike(*event_archive_set))
                                   >> std::tie(event_archive_set->id);
                    }
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct ChangeImpl<AdapterPtr, EventArchiveSettingsPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<EventArchiveSettingsPtr> event_archive_settings)
    {
        LOG(info) << "Update event_archive_settings from DB";
        std::for_each(event_archive_settings.begin(),
                      event_archive_settings.end(),
                      [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
            {
                [&]
                {
                    for (const auto& event_archive_set: event_archive_settings)
                    {
                        (*adapter) << (Update | EventArchiveSettingsTable
                                              | From(*event_archive_set)
                                              | Where((boost::format("id = %1%") % event_archive_set->id).str()))
                                   >> ExecuteEmpty{};
                    };
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct RemoveImpl<AdapterPtr, EventArchiveSettingsPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<EventArchiveSettingsPtr> event_archive_settings)
    {
        LOG(info) << "Remove event_archive_settings from DB";
        std::for_each(event_archive_settings.begin(),
                      event_archive_settings.end(),
                      [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
            {
                [&]
                {
                    for (const auto& event_archive_set: event_archive_settings)
                    {
                        (*adapter) << (DeleteFrom | EventArchiveSettingsTable
                                                  | Where((boost::format("id = %1%") % event_archive_set->id).str()))
                                   >> ExecuteEmpty{};
                    }
                }
            }.result();
    }
};

}

#endif //CAMERAMANAGERCORE_SQLITEEVENTARCHIVESETTINGSSTATEMENT_HPP
