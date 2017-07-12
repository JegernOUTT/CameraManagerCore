//
// Created by svakhreev on 09.03.17.
//

#ifndef CAMERAMANAGERCORE_JSONPARCER_HPP
#define CAMERAMANAGERCORE_JSONPARCER_HPP

#include <iostream>
#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <rttr/type>

namespace cameramanagercore::model::parcers
{

void to_json_recursively(const rttr::instance& obj, nlohmann::json& j);
void fromjson_recursively(rttr::instance obj, const nlohmann::json& j);
bool equal_recursively(const rttr::instance& lhs, const rttr::instance& rhs);


void write_atomic_types_to_json(const rttr::type& t, const rttr::variant& var, nlohmann::json& j);
void write_array(const rttr::variant_sequential_view& a, nlohmann::json& j);
void to_json_recursively(const rttr::instance& obj2, nlohmann::json& j);
nlohmann::json impl_to_json(rttr::instance instance);

template<typename T>
nlohmann::json impl_to_json(const std::vector<T>& objects)
{
    nlohmann::json j = nlohmann::json::array();
    for (const auto& item: objects)
    {
        rttr::instance inst = item;
        if (!inst.is_valid()) continue;
        nlohmann::json json_object = nlohmann::json::object();
        to_json_recursively(std::move(inst), json_object);
        j.push_back(move(json_object));
    }

    return j;
}


rttr::variant extract_basic_types(const nlohmann::json& json_value);
void write_array_recursively(rttr::variant_sequential_view& var_array, const nlohmann::json& json_array_value);
void fromjson_recursively(rttr::instance obj2, const nlohmann::json& j);
void impl_from_json(const nlohmann::json& j, rttr::instance value);


bool equal_atomic_types(const rttr::type& t, const rttr::variant& lhs, const rttr::variant& rhs);
bool equal_array(const rttr::variant_sequential_view& lhs, const rttr::variant_sequential_view& rhs);
bool equal_recursively(const rttr::instance& lhs, const rttr::instance& rhs);
}

namespace cameramanagercore::model
{

template<typename T>
void to_json(nlohmann::json& j, const T& p)
{
    j = parcers::impl_to_json(p);
}

template<typename T>
void to_json(nlohmann::json& j, const std::vector<T>& p)
{
    j = parcers::impl_to_json(p);
}

template<typename T>
void from_json(const nlohmann::json& j, T& p)
{
    parcers::impl_from_json(j, p);
}

}

#endif //CAMERAMANAGERCORE_JSONPARCER_HPP
