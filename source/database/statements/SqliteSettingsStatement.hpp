//
// Created by svakhreev on 20.02.17.
//

#ifndef CAMERAMANAGERCORE_SQLITESETTINGSSTATEMENT_HPP
#define CAMERAMANAGERCORE_SQLITESETTINGSSTATEMENT_HPP

#include <vector>
#include <memory>
#include <initializer_list>
#include <tuple>

#include <boost/format.hpp>

#include "../statements/Statements.hpp"
#include "../adapters/SqlLanguageAdapter.hpp"
#include "../../model/Settings.hpp"

namespace cameramanagercore::database::statements
{

using namespace adapters;
using namespace cameramanagercore::model;

using SettingsPtr = std::shared_ptr<Settings>;

template<typename AdapterPtr>
struct CreateTableImpl<AdapterPtr, SettingsPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Creating settings table";

        return ErrorHandler
            {
                [&]
                {
                    (*adapter) << "CREATE TABLE IF NOT EXISTS settings (\n"
                                  "    id              INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                                  "    registration    BLOB,\n"
                                  "    http_port       INTEGER NOT NULL,\n"
                                  "    ws_camera_port  INTEGER NOT NULL,\n"
                                  "    ws_model_port   INTEGER NOT NULL,\n"
                                  "    ws_events_port  INTEGER NOT NULL\n"
                                  ");"
                               >> ExecuteEmpty{};;
                }
            }.result();
    }
};

template<typename AdapterPtr>
struct GetImpl<AdapterPtr, SettingsPtr>
{
    auto operator()(AdapterPtr adapter)
    {
        LOG(info) << "Get settings from DB";

        return ErrorHandlerWithData<std::vector<SettingsPtr>>
        {
            [&]
            {
                std::vector<SettingsPtr> settings;
                auto visitor = [&](int id,
                                   std::vector<int> registration,
                                   int http_port,
                                   int ws_cameras_port,
                                   int ws_model_port,
                                   int ws_events_port)
                { settings.emplace_back(std::make_shared<Settings>(Settings{ registration,
                                                                             http_port,
                                                                             ws_cameras_port,
                                                                             ws_model_port,
                                                                             ws_events_port,
                                                                             id })); };
                (*adapter) << (SelectFrom | SettingsTable)
                           >> visitor;
                return settings;
            }
        }.result();


    }
};

template<typename AdapterPtr>
struct AddImpl<AdapterPtr, SettingsPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<SettingsPtr> settings)
    {
        LOG(info) << "Add new settings to DB";
        std::for_each(settings.begin(), settings.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& setting: settings)
                {
                    (*adapter) << (InsertTo | SettingsTable | Values(*setting))
                               << AddBlob { setting->registration }
                               >> ExecuteEmpty{}
                               << (SelectFrom | SettingsTable | WhereRowLike(*setting))
                               << AddBlob { setting->registration }
                               >> std::tie(setting->id);
                }
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct ChangeImpl<AdapterPtr, SettingsPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<SettingsPtr> settings)
    {
        LOG(info) << "Update settings from DB";
        std::for_each(settings.begin(), settings.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& setting: settings)
                {
                    (*adapter) << (Update | SettingsTable | From(*setting)
                                          | Where((boost::format("id = %1%") % setting->id).str()))
                               << AddBlob { setting->registration }
                               >> ExecuteEmpty{};
                };
            }
        }.result();
    }
};

template<typename AdapterPtr>
struct RemoveImpl<AdapterPtr, SettingsPtr>
{
    auto operator()(AdapterPtr adapter, std::initializer_list<SettingsPtr> settings)
    {
        LOG(info) << "Remove settings from DB";
        std::for_each(settings.begin(), settings.end(), [](auto&& ptr) { LOG(info) << *ptr; });

        return ErrorHandler
        {
            [&]
            {
                for (const auto& setting: settings)
                {
                    (*adapter) << (DeleteFrom | SettingsTable
                                              | Where((boost::format("id = %1%") % setting->id).str()))
                               >> ExecuteEmpty{};
                }
            }
        }.result();
    }
};

}

#endif //CAMERAMANAGERCORE_SQLITESETTINGSSTATEMENT_HPP
