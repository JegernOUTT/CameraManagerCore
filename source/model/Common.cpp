//
// Created by svakhreev on 09.03.17.
//

#include <rttr/registration>
#include <nlohmann/json.hpp>

namespace cameramanagercore::model
{

namespace impl
{
using namespace rttr;
using namespace nlohmann;

bool _impl_equal_recursively(const instance& lhs, const instance& rhs);
bool _impl_equal_atomic_types(const type& t, const variant& lhs, const variant& rhs)
{
    if (t.is_arithmetic())
    {
        if (t == type::get<bool>())
            return lhs.to_bool() == rhs.to_bool();
        else if (t == type::get<char>())
            return lhs.to_bool() == rhs.to_bool();
        else if (t == type::get<int8_t>())
            return lhs.to_int8() == rhs.to_int8();
        else if (t == type::get<int16_t>())
            return lhs.to_int16() == rhs.to_int16();
        else if (t == type::get<int32_t>())
            return lhs.to_int32() == rhs.to_int32();
        else if (t == type::get<int64_t>())
            return lhs.to_int64() == rhs.to_int64();
        else if (t == type::get<uint8_t>())
            return lhs.to_uint8() == rhs.to_uint8();
        else if (t == type::get<uint16_t>())
            return lhs.to_uint16() == rhs.to_uint16();
        else if (t == type::get<uint32_t>())
            return lhs.to_uint32() == rhs.to_uint32();
        else if (t == type::get<uint64_t>())
            return lhs.to_uint64() == rhs.to_uint64();
        else if (t == type::get<float>())
            return lhs.to_double() == rhs.to_double();
        else if (t == type::get<double>())
            return lhs.to_double() == rhs.to_double();
    }
    else if (t.is_enumeration())
    {
        return lhs.to_int() == rhs.to_int();
    }
    else if (t == type::get<std::string>())
    {
        return lhs.to_string() == rhs.to_string();
    }

    return true;
}
bool _impl_equal_array(const variant_sequential_view& lhs, const variant_sequential_view& rhs)
{
    if (lhs.get_size() != rhs.get_size()) return false;

    for (size_t i = 0; i < lhs.get_size(); ++i)
    {
        variant lhs_var = lhs.get_value(i);
        variant rhs_var = rhs.get_value(i);
        if (lhs_var.is_sequential_container())
        {
            return _impl_equal_array(lhs_var.create_sequential_view(),
                                     rhs_var.create_sequential_view());
        }
        else
        {
            variant lhs_wrapped_var = lhs_var.extract_wrapped_value();
            variant rhs_wrapped_var = rhs_var.extract_wrapped_value();
            type value_type = lhs_wrapped_var.get_type();

            if (value_type.is_arithmetic()
                || value_type == type::get<std::string>()
                || value_type.is_enumeration())
            {
                return _impl_equal_atomic_types(value_type, lhs_wrapped_var, rhs_wrapped_var);
            }
            else
            {
                return _impl_equal_recursively(lhs_wrapped_var, rhs_wrapped_var);
            }
        }
    }

    return true;
}
bool _impl_equal_recursively(const instance& lhs, const instance& rhs)
{
    instance lhs_obj = lhs.get_type().is_wrapper() ? lhs.get_wrapped_instance() : lhs;
    instance rhs_obj = rhs.get_type().is_wrapper() ? rhs.get_wrapped_instance() : rhs;

    auto prop_list = lhs_obj.get_derived_type().get_properties();
    for (auto prop : prop_list)
    {
        bool is_equal = true;

        variant lhs_prop_value = prop.get_value(lhs_obj);
        variant rhs_prop_value = prop.get_value(rhs_obj);
        if (!lhs_prop_value || !rhs_prop_value) continue;

        std::string name = prop.get_name().data();
        type value_type = lhs_prop_value.get_type();
        if (value_type.is_arithmetic()
            || value_type == type::get<std::string>()
            || value_type.is_enumeration())
        {
            is_equal = _impl_equal_atomic_types(value_type, lhs_prop_value, rhs_prop_value) && is_equal;
        }
        else if (value_type.is_sequential_container())
        {
            is_equal =  _impl_equal_array(lhs_prop_value.create_sequential_view(),
                                          rhs_prop_value.create_sequential_view()) && is_equal;
        }
        else
        {
            auto child_props = value_type.get_properties();
            if (!child_props.empty())
                is_equal = _impl_equal_recursively(lhs_prop_value, rhs_prop_value) && is_equal;
            else
                is_equal = lhs_prop_value.to_string() == rhs_prop_value.to_string() && is_equal;
        }

        if (!is_equal) return false;
    }
    return true;
}
}

}


