//
// Created by svakhreev on 28.12.16.
//

#ifndef CAMERAMANAGERCORE_SQLLANGUAGEADAPTER_HPP
#define CAMERAMANAGERCORE_SQLLANGUAGEADAPTER_HPP

#include <functional>
#include <string>
#include <memory>
#include <numeric>
#include <nlohmann/json.hpp>
#include <rttr/registration>

#include <boost/format.hpp>
#include <range/v3/all.hpp>

namespace cameramanagercore::database::adapters {

using namespace std::string_literals;
using namespace rttr;

auto get_data_insert = [](const auto& ref)
{
    using namespace ranges;
    auto t = rttr::type::get<std::remove_reference_t<decltype(ref)>>();
//    auto temp_properties = t.get_properties();
//
//    std::vector<rttr::property> properties = temp_properties
//                                             | view::filter([] (auto&& i) { return i.get_name() != "id"s; });
//    std::vector<rttr::property> last_property = properties | view::drop(properties.size() - 1);
//
//    std::string fields = properties | view::take(properties.size() - 1)
//                                    | view::transform([] (auto&& i) { return i.get_name() + ", "s; })
//                                    | view::join;
//
//    fields += last_property | view::transform([] (auto&& i) { return i.get_name(); })
//                            | view::join;
//
//    auto value_get = [&t, &ref] (auto&& property)
//    {
//        std::string value;
//        if (property.get_type().is_enumeration())
//        {
//            return (boost::format("%1%") % property.get_value(ref).to_int()).str();
//        }
//        else if (property.get_type() == rttr::type::get<std::string>())
//        {
//            return (boost::format("'%1%'") % property.get_value(ref).template get_value<std::string>()).str();
//        }
//        else if (property.get_type() == rttr::type::get<bool>())
//        {
//            return std::to_string(property.get_value(ref).to_int());
//        }
//        else if (property.is_array())
//        {
//            return "?"s;
//        }
//        else
//        {
//            return property.get_value(ref).to_string();
//        }
//    };
//
//    std::string values = properties | view::take(properties.size() - 1)
//                                    | view::transform(value_get)
//                                    | view::transform([] (auto&& i) { return i + ", "s; })
//                                    | view::join;
//
//    values += last_property | view::transform(value_get)
//                            | view::join;
//
//    auto f = boost::format("(%1%) VALUES (%2%)") % fields % values;

    auto temp_properties = t.get_properties();
    std::vector<rttr::property> properties = temp_properties
                                             | view::filter([] (auto&& i) { return i.get_name() != "id"s; });

    auto fields =  std::accumulate(properties.begin(),
                                   properties.end(),
                                   ""s,
                                   [&properties] (auto& result, auto& property)
                                   {
                                       return (*properties.rbegin()) == property
                                              ?
                                       result += std::string { property.get_name() }
                                                                                       :
                                       result += (std::string { property.get_name() } + ", ");
                                   });

    auto f = boost::format("(%1%) VALUES (%2%)")
             % fields
             % std::accumulate(properties.begin(),
                               properties.end(),
                               ""s,
                               [&properties, &ref] (auto& result, auto& property)
                               {
                                   std::string value {};
                                   if (property.get_type().is_enumeration())
                                   {
                                       value = (boost::format("%1%") % property.get_value(ref).to_int()).str();
                                   }
                                   else if (property.get_type() == rttr::type::get<std::string>())
                                   {
                                       value = (boost::format("'%1%'") % property.get_value(ref).template get_value<std::string>()).str();
                                   }
                                   else if (property.get_type() == rttr::type::get<bool>())
                                   {
                                       value = std::to_string(property.get_value(ref).to_int());
                                   }
                                   else if (property.is_array())
                                   {
                                       value = "?";
                                   }
                                   else
                                   {
                                       value = property.get_value(ref).to_string();
                                   }

                                   return (*properties.rbegin()) == property ?
                                       result += value
                                                                                     :
                                       result += value + ", ";
                               });
    return f.str();
};

auto get_values_update = [](const auto& ref)
{
    using namespace ranges;
    auto t = rttr::type::get<std::remove_reference_t<decltype(ref)>>();

    auto temp_properties = t.get_properties();
    std::vector<rttr::property> properties = temp_properties
                                             | view::filter([] (auto&& i) { return i.get_name() != "id"s; });

    return std::accumulate(properties.begin(),
                           properties.end(),
                           ""s,
                           [&properties, &ref] (auto& result, auto& property)
                           {
                               auto name = property.get_name();
                               if (name == "id") return result;

                               std::string value {};
                               if (property.get_type().is_enumeration())
                               {
                                   value = (boost::format("%1%") % property.get_value(ref).to_int()).str();
                               }
                               else if (property.get_type() == rttr::type::get<std::string>())
                               {
                                   value = (boost::format("'%1%'") % property.get_value(ref).to_string()).str();
                               }
                               else if (property.get_type() == rttr::type::get<bool>())
                               {
                                   value = std::to_string(property.get_value(ref).to_int());
                               }
                               else if (property.is_array())
                               {
                                   value = "?";
                               }
                               else
                               {
                                   value = property.get_value(ref).to_string();
                               }
                               return (*properties.rbegin()) == property ?
                                   result += (boost::format("%1% = %2%") % name % value).str()
                                                                                 :
                                   result += (boost::format("%1% = %2%, ") % name % value).str();
                           });
};

auto get_values_whereand = [](const auto& ref)
{
    using namespace ranges;
    auto t = rttr::type::get<std::remove_reference_t<decltype(ref)>>();

    auto temp_properties = t.get_properties();
    std::vector<rttr::property> properties = temp_properties
                                             | view::filter([] (auto&& i) { return i.get_name() != "id"s; });

    return std::accumulate(properties.begin(),
                           properties.end(),
                           ""s,
                           [&properties, &ref] (auto& result, auto& property)
                           {
                               auto name = property.get_name();
                               if (name == "id") return result;

                               std::string value {};
                               if (property.get_type().is_enumeration())
                               {
                                   value = (boost::format("%1%") % property.get_value(ref).to_int()).str();
                               }
                               else if (property.get_type() == rttr::type::get<std::string>())
                               {
                                   value = (boost::format("'%1%'") % property.get_value(ref).to_string()).str();
                               }
                               else if (property.get_type() == rttr::type::get<bool>())
                               {
                                   value = std::to_string(property.get_value(ref).to_int());
                               }
                               else if (property.is_array())
                               {
                                   value = "?";
                               }
                               else
                               {
                                   value = property.get_value(ref).to_string();
                               }
                               return (*properties.rbegin()) == property ?
                                   result += (boost::format("%1% = %2%") % name % value).str()
                                                                                 :
                                   result += (boost::format("%1% = %2% AND ") % name % value).str();
                           });
};

std::string operator | (std::function<boost::format()>&& f1, std::function<std::string()>&& f2);
std::string operator | (boost::format f1, std::function<std::string()>&& f2);
std::string operator | (std::function<std::string()>&& f1, std::function<std::string()>&& f2);
std::string operator | (std::string s, std::function<std::string()>&& f2);
std::string operator | (std::function<std::string()>&& f1, std::string s);
std::string operator | (std::string s1, std::string s2);

auto SelectFrom =            [] { return boost::format("SELECT * FROM %1%"); };
auto SelectSomeFrom =        [] (std::string args) { return boost::format("SELECT %1% FROM %2%") % args; };

auto InsertTo =              [] { return boost::format("INSERT INTO %1%"); };
auto Values =                [] (auto&& object) { return (boost::format("%1%;") %
                                                  get_data_insert(std::forward<decltype(object)>(object))).str(); };
auto ValuesManual =          [] (std::string condition) { return (boost::format("%1%;") % condition).str(); };

auto Update =                [] { return boost::format("UPDATE %1%"); };
auto From =                  [] (auto&& object) { return (boost::format("SET %1%") %
                                                          get_values_update(std::forward<decltype(object)>(object))).str(); };
auto FromManual =            [] (std::string condition) { return (boost::format("SET %1%") % condition).str(); };

auto DeleteFrom =            [] { return boost::format("DELETE FROM %1%"); };

auto Where =                 [] (std::string condition) { return (boost::format("WHERE %1%;") % condition).str(); };
auto WhereRowLike =          [] (auto&& object) { return (boost::format("WHERE %1%;") %
                                                  get_values_whereand(std::forward<decltype(object)>(object))).str(); };

auto UserTable =                  [] { return "user"s; };
auto CameraTable =                [] { return "camera"s; };
auto EventTable =                 [] { return "event"s; };
auto PermissionTable =            [] { return "permission"s; };
auto SettingsTable =              [] { return "settings"s; };
auto UserCamerasTable =           [] { return "user_cameras"s; };
auto UserPermissionsTable =       [] { return "user_permissions"s; };
auto SessionsTable =              [] { return "sessions"s; };
auto CameraActionTable =          [] { return "camera_action"s; };
auto EventArchiveSettingsTable =  [] { return "event_archive_settings"s; };

}

#endif //CAMERAMANAGERCORE_SQLLANGUAGEADAPTER_HPP
