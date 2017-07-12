//
// Created by svakhreev on 28.03.17.
//

#ifndef CAMERAMANAGERCORE_UTILITY_HPP
#define CAMERAMANAGERCORE_UTILITY_HPP

#include <tuple>
#include <memory>
#include <utility>
#include <initializer_list>
#include <string_view>
#include <type_traits>

namespace cameramanagercore::schemas
{

template < std::size_t CurrentIndex,
           typename CurrentType,
           template <typename...> typename... Args,
           typename... Package >
constexpr auto decorate_impl(CurrentType current, Package&&... package)
{
    if constexpr (CurrentIndex < sizeof...(Args))
    {
        using NewType = typename std::tuple_element<CurrentIndex, std::tuple<Args< std::decay_t<Package>...,
                                                                                   std::decay_t<CurrentType >>...>>::type;
        auto new_current = NewType { std::forward<Package>(package)..., current };
        return decorate_impl<CurrentIndex + 1, NewType, Args...>(new_current, std::forward<Package>(package)...);
    }
    else
    {
        return current;
    }
}

template < template <typename...> typename... Args,
           typename Functor,
           typename... Package >
constexpr auto decorate_with(Functor func, Package&&... package)
{
    static_assert(sizeof...(Args) > 0, "Decorating must contains >0 of types");
    return decorate_impl<0, Functor, Args...>(func,
                                              std::forward<Package>(package)...);
}

template < template <typename...> typename... Decorators,
           typename Server,
           typename Functor,
           typename Contexts >
void set_resource(Server& server,
                  std::string_view uri,
                  std::initializer_list<std::string_view> methods,
                  Functor func,
                  Contexts&& contexts)
{
    for (auto&& method: methods)
    {
        server->resource[uri.data()][method.data()] =
            decorate_with < Decorators... > (func, std::forward<Contexts>(contexts));
    }

}

}

#endif //CAMERAMANAGERCORE_UTILITY_HPP
