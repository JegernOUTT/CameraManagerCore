//
// Created by svakhreev on 27.03.17.
//

#ifndef CAMERAMANAGERCORE_PARCERS_HPP
#define CAMERAMANAGERCORE_PARCERS_HPP

#include <iostream>
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>

#include "../../model/parcers/JsonParcer.hpp"
#include "../../model/detail/Detectors.hpp"
#include "../../model/Common.hpp"

namespace cameramanagercore::schemas::model
{

namespace impl
{
bool _impl_equal_recursively(const rttr::instance& lhs, const rttr::instance& rhs);
bool _impl_equal_atomic_types(const rttr::type& t, const rttr::variant& lhs, const rttr::variant& rhs);
bool _impl_equal_array(const rttr::variant_sequential_view& lhs, const rttr::variant_sequential_view& rhs);
bool _impl_equal_recursively(const rttr::instance& lhs, const rttr::instance& rhs);
}

template<typename T>
static cameramanagercore::model::detail::enable_flat_not_eq_comp_t<T> operator == (const T& lhs, const T& rhs) noexcept
{
    return impl::_impl_equal_recursively(lhs, rhs);
}

template<typename T>
static cameramanagercore::model::detail::enable_flat_not_ne_comp_t<T> operator != (const T& lhs, const T& rhs) noexcept
{
    return !impl::_impl_equal_recursively(lhs, rhs);
}

template<typename T>
void to_json(nlohmann::json& j, const T& p)
{
    j = cameramanagercore::model::parcers::impl_to_json(p);
}

template<typename T>
void to_json(nlohmann::json& j, const std::vector<T>& p)
{
    j = cameramanagercore::model::parcers::impl_to_json(p);
}

template<typename T>
void from_json(const nlohmann::json& j, T& p)
{
    cameramanagercore::model::parcers::impl_from_json(j, p);
}
}

#endif //CAMERAMANAGERCORE_PARCERS_HPP
