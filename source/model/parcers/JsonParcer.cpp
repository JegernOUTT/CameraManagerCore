//
// Created by svakhreev on 09.03.17.
//
#include <cstdio>
#include <vector>
#include <array>
#include <iostream>

#include <nlohmann/json.hpp>
#include <rttr/type>

namespace cameramanagercore::model::parcers
{

void to_json_recursively(const rttr::instance& obj, nlohmann::json& j);
void fromjson_recursively(rttr::instance obj, const nlohmann::json& j);
bool equal_recursively(const rttr::instance& lhs, const rttr::instance& rhs);
nlohmann::json impl_to_json(rttr::instance instance)
{
    if (!instance.is_valid())
        return std::string();

    nlohmann::json j = nlohmann::json::object();
    to_json_recursively(instance, j);

    return j;
}


void write_atomic_types_to_json(const rttr::type& t, const rttr::variant& var, nlohmann::json& j)
{
    if (t.is_arithmetic())
    {
        if (t == rttr::type::get<bool>())
            j = var.to_bool();
        else if (t == rttr::type::get<char>())
            j = var.to_bool();
        else if (t == rttr::type::get<int8_t>())
            j = var.to_int8();
        else if (t == rttr::type::get<int16_t>())
            j = var.to_int16();
        else if (t == rttr::type::get<int32_t>())
            j = var.to_int32();
        else if (t == rttr::type::get<int64_t>())
            j = var.to_string();
        else if (t == rttr::type::get<uint8_t>())
            j = var.to_uint8();
        else if (t == rttr::type::get<uint16_t>())
            j = var.to_uint16();
        else if (t == rttr::type::get<uint32_t>())
            j = var.to_uint32();
        else if (t == rttr::type::get<uint64_t>())
            j = var.to_string();
        else if (t == rttr::type::get<float>())
            j = var.to_double();
        else if (t == rttr::type::get<double>())
            j = var.to_double();
    }
    else if (t.is_enumeration())
    {
        bool ok = false;
        auto result = var.to_string(&ok);
        if (ok)
        {
            j = var.to_string();
            return;
        }
        ok = false;
        auto value = var.to_uint64(&ok);
        if (ok)
            j = value;
        else
            j = nlohmann::json({});
    }
    else if (t == rttr::type::get<std::string>())
    {
        j = var.to_string();
    }
}

void write_array(const rttr::variant_sequential_view& a, nlohmann::json& j)
{
    for (size_t i = 0; i < a.get_size(); ++i)
    {
        rttr::variant var = a.get_value(i);
        if (var.is_sequential_container())
        {
            write_array(var.create_sequential_view(), j);
        }
        else
        {
            rttr::variant wrapped_var = var.extract_wrapped_value();
            rttr::type value_type = wrapped_var.get_type();
            if (value_type.is_arithmetic()
                || value_type == rttr::type::get<std::string>()
                || value_type.is_enumeration())
            {
                auto json_object = nlohmann::json::object();
                write_atomic_types_to_json(value_type, wrapped_var, json_object);
                j.push_back(std::move(json_object));
            }
            else
            {
                auto json_object = nlohmann::json::object();
                to_json_recursively(wrapped_var, json_object);
                j.push_back(std::move(json_object));
            }
        }
    }
}

void to_json_recursively(const rttr::instance& obj2, nlohmann::json& j)
{
    rttr::instance obj = obj2.get_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;

    auto prop_list = obj.get_derived_type().get_properties();
    for (auto prop : prop_list)
    {
        if (prop.get_metadata("NO_SERIALIZE"))
            continue;

        rttr::variant prop_value = prop.get_value(obj);
        if (!prop_value)
            continue; // cannot serialize, because we cannot retrieve the value

        std::string name = prop.get_name().data();
        rttr::type value_type = prop_value.get_type();
        if (value_type.is_arithmetic()
            || value_type == rttr::type::get<std::string>()
            || value_type.is_enumeration())
        {
            auto json_object = nlohmann::json::object();
            write_atomic_types_to_json(value_type, prop_value, json_object);
            j[name] = std::move(json_object);
        }
        else if (value_type.is_sequential_container())
        {
            auto json_array = nlohmann::json::array();
            write_array(prop_value.create_sequential_view(), json_array);
            j[name] = std::move(json_array);
        }
        else
        {
            auto json_object = nlohmann::json::object();
            auto child_props = value_type.get_properties();
            if (!child_props.empty())
            {
                to_json_recursively(prop_value, json_object);
                j[name] = std::move(json_object);
            }
            else
            {
                auto text = prop_value.to_string();
                j[name] = text;
            }
        }
    }
}



rttr::variant extract_basic_types(const nlohmann::json& json_value)
{
    switch(json_value.type())
    {
        case nlohmann::json::value_t::string:
            return std::string(json_value.get<std::string>());
        case nlohmann::json::value_t::null:
            break;
        case nlohmann::json::value_t::boolean:
            return json_value.get<bool>();
        case nlohmann::json::value_t::number_integer:
            return json_value.get<int>();
        case nlohmann::json::value_t::number_float:
            return json_value.get<double>();
        case nlohmann::json::value_t::number_unsigned:
            return json_value.get<uint32_t>();
        default:
            return {};
    }

    return rttr::variant();
}

void write_array_recursively(rttr::variant_sequential_view& var_array, const nlohmann::json& json_array_value)
{
    var_array.set_size(json_array_value.size());
    for (size_t i = 0; i < json_array_value.size(); ++i)
    {
        const nlohmann::json& json_index_value = json_array_value[i];
        if (json_index_value.is_array())
        {
            auto sub_array_view = var_array.get_value(i).create_sequential_view();
            write_array_recursively(sub_array_view, json_index_value);
        }
        else if (json_index_value.is_object())
        {
            rttr::variant var_tmp = var_array.get_value(i);
            rttr::variant wrapped_var = var_tmp.extract_wrapped_value();
            fromjson_recursively(wrapped_var, json_index_value);
            var_array.set_value(i, wrapped_var);
        }
        else
        {
            const rttr::type array_type = var_array.get_rank_type(i);
            rttr::variant extracted_value = extract_basic_types(json_index_value);
            if (extracted_value.convert(array_type))
                var_array.set_value(i, extracted_value);
        }
    }
}

// Если пришёл json массив, то забираю только первый элемент
void fromjson_recursively(rttr::instance obj2, const nlohmann::json& j)
{
    rttr::instance obj = obj2.get_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;
    const auto prop_list = obj.get_derived_type().get_properties();
    const nlohmann::json& js = j.is_array() && j.size() > 0 ? j[0] : j;

    for (auto prop : prop_list)
    {
        const rttr::type value_t = prop.get_type();
        if (js.find(prop.get_name().data()) == js.cend()) continue;
        const nlohmann::json& json_value = js[prop.get_name().data()];
        switch(json_value.type())
        {
            case nlohmann::json::value_t::array:
            {
                rttr::variant var = prop.get_value(obj);
                auto array_view = var.create_sequential_view();
                write_array_recursively(array_view, json_value);
                prop.set_value(obj, var);
                break;
            }
            case nlohmann::json::value_t::object:
            {
                rttr::variant var = prop.get_value(obj);
                fromjson_recursively(var, json_value);
                prop.set_value(obj, var);
                break;
            }
            default:
            {
                rttr::variant extracted_value = extract_basic_types(json_value);
                if (extracted_value.convert(value_t))
                    prop.set_value(obj, extracted_value);
            }
        }
    }
}

void impl_from_json(const nlohmann::json& j, rttr::instance value)
{
    fromjson_recursively(value, j);
}



bool equal_atomic_types(const rttr::type& t, const rttr::variant& lhs, const rttr::variant& rhs)
{
    if (t.is_arithmetic())
    {
        if (t == rttr::type::get<bool>())
            return lhs.to_bool() == rhs.to_bool();
        else if (t == rttr::type::get<char>())
            return lhs.to_bool() == rhs.to_bool();
        else if (t == rttr::type::get<int8_t>())
            return lhs.to_int8() == rhs.to_int8();
        else if (t == rttr::type::get<int16_t>())
            return lhs.to_int16() == rhs.to_int16();
        else if (t == rttr::type::get<int32_t>())
            return lhs.to_int32() == rhs.to_int32();
        else if (t == rttr::type::get<int64_t>())
            return lhs.to_int64() == rhs.to_int64();
        else if (t == rttr::type::get<uint8_t>())
            return lhs.to_uint8() == rhs.to_uint8();
        else if (t == rttr::type::get<uint16_t>())
            return lhs.to_uint16() == rhs.to_uint16();
        else if (t == rttr::type::get<uint32_t>())
            return lhs.to_uint32() == rhs.to_uint32();
        else if (t == rttr::type::get<uint64_t>())
            return lhs.to_uint64() == rhs.to_uint64();
        else if (t == rttr::type::get<float>())
            return lhs.to_double() == rhs.to_double();
        else if (t == rttr::type::get<double>())
            return lhs.to_double() == rhs.to_double();
    }
    else if (t.is_enumeration())
    {
        return lhs.to_int() == rhs.to_int();
    }
    else if (t == rttr::type::get<std::string>())
    {
        return lhs.to_string() == rhs.to_string();
    }

    return false;
}

bool equal_array(const rttr::variant_sequential_view& lhs, const rttr::variant_sequential_view& rhs)
{
    if (lhs.get_size() != rhs.get_size()) return false;

    for (size_t i = 0; i < lhs.get_size(); ++i)
    {
        rttr::variant lhs_var = lhs.get_value(i);
        rttr::variant rhs_var = rhs.get_value(i);
        if (lhs_var.is_sequential_container())
        {
            return equal_array(lhs_var.create_sequential_view(),
                               rhs_var.create_sequential_view());
        }
        else
        {
            rttr::variant lhs_wrapped_var = lhs_var.extract_wrapped_value();
            rttr::variant rhs_wrapped_var = rhs_var.extract_wrapped_value();
            rttr::type value_type = lhs_wrapped_var.get_type();

            if (value_type.is_arithmetic()
                || value_type == rttr::type::get<std::string>()
                || value_type.is_enumeration())
            {
                return equal_atomic_types(value_type, lhs_wrapped_var, rhs_wrapped_var);
            }
            else
            {
                return equal_recursively(lhs_wrapped_var, rhs_wrapped_var);
            }
        }
    }
    return false;
}

bool equal_recursively(const rttr::instance& lhs, const rttr::instance& rhs)
{
    rttr::instance lhs_obj = lhs.get_type().is_wrapper() ? lhs.get_wrapped_instance() : lhs;
    rttr::instance rhs_obj = rhs.get_type().is_wrapper() ? rhs.get_wrapped_instance() : rhs;

    auto prop_list = lhs_obj.get_derived_type().get_properties();
    for (auto prop : prop_list)
    {
        bool is_equal = true;

        rttr::variant lhs_prop_value = prop.get_value(lhs_obj);
        rttr::variant rhs_prop_value = prop.get_value(rhs_obj);
        if (!lhs_prop_value || !rhs_prop_value) continue;

        std::string name = prop.get_name().data();
        rttr::type value_type = lhs_prop_value.get_type();
        if (value_type.is_arithmetic()
            || value_type == rttr::type::get<std::string>()
            || value_type.is_enumeration())
        {
            is_equal = equal_atomic_types(value_type, lhs_prop_value, rhs_prop_value) && is_equal;
        }
        else if (value_type.is_sequential_container())
        {
            is_equal =  equal_array(lhs_prop_value.create_sequential_view(),
                                    rhs_prop_value.create_sequential_view()) && is_equal;
        }
        else
        {
            auto child_props = value_type.get_properties();
            if (!child_props.empty())
                is_equal = equal_recursively(lhs_prop_value, rhs_prop_value) && is_equal;
            else
                is_equal = lhs_prop_value.to_string() == rhs_prop_value.to_string() && is_equal;
        }

        if (!is_equal) return false;
    }
    return true;
}

}
