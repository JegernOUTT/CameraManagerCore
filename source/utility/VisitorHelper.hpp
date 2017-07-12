//
// Created by svakhreev on 15.02.17.
//

#ifndef CAMERAMANAGERCORE_VISITORHELPER_HPP
#define CAMERAMANAGERCORE_VISITORHELPER_HPP

#include <type_traits>
#include <utility>
#include <memory>
#include <variant>
#include <boost/variant.hpp>

namespace cameramanagercore::utility
{

template<typename... Args>
struct VisitorBase : Args...
{
    VisitorBase (Args&&... args)
        : Args { std::forward<Args> (args) }... { }
};

template<typename R, typename... Args>
struct Visitor : boost::static_visitor<R>, VisitorBase<Args...>
{
    using VisitorBase<Args...>::VisitorBase;
};

template<typename HeadVar, typename... TailVars, typename... Args>
auto Visit (std::variant<HeadVar, TailVars...>& v, Args&&... args)
{
    using R_t = decltype (VisitorBase<std::remove_reference_t<Args>...> { std::forward<Args> (args)... } (std::declval<HeadVar> ()));
    return std::visit(Visitor<R_t, Args...> { std::forward<Args> (args)... }, v);
}

}

#endif //CAMERAMANAGERCORE_VISITORHELPER_HPP
