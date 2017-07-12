//
// Created by svakhreev on 02.03.17.
//

#ifndef CAMERAMANAGERCORE_COMMON_HPP_HPP
#define CAMERAMANAGERCORE_COMMON_HPP_HPP

#include <ostream>
#include <vector>
#include <string>
#include <type_traits>
#include <type_traits>
#include <rttr/registration>
#include <nlohmann/json.hpp>

#include "detail/Detectors.hpp"
#include "parcers/JsonParcer.hpp"

namespace cameramanagercore::model
{

namespace impl
{
bool _impl_equal_recursively(const rttr::instance& lhs, const rttr::instance& rhs);
bool _impl_equal_atomic_types(const rttr::type& t, const rttr::variant& lhs, const rttr::variant& rhs);
bool _impl_equal_array(const rttr::variant_sequential_view& lhs, const rttr::variant_sequential_view& rhs);
bool _impl_equal_recursively(const rttr::instance& lhs, const rttr::instance& rhs);
}

template<typename T>
static detail::enable_flat_not_eq_comp_t<T> operator == (const T& lhs, const T& rhs) noexcept
{
    return impl::_impl_equal_recursively(lhs, rhs);
}

template<typename T>
static detail::enable_flat_not_ne_comp_t<T> operator != (const T& lhs, const T& rhs) noexcept
{
    return !impl::_impl_equal_recursively(lhs, rhs);
}

template <typename Char, typename Traits, class T>
static detail::enable_flat_not_ostreamable_t<std::basic_ostream<Char, Traits>, T>
       operator << (std::basic_ostream<Char, Traits>& stream, const T& value) noexcept
{
    nlohmann::json j = value;
    return stream << j;
}

}

#endif //CAMERAMANAGERCORE_COMMON_HPP_HPP
