//
// Created by svakhreev on 17.03.17.
//

#ifndef CAMERAMANAGERCORE_TUPLEUTILITY_HPP
#define CAMERAMANAGERCORE_TUPLEUTILITY_HPP

#include <tuple>
#include <type_traits>

namespace cameramanagercore::utility
{

template < typename T,
           T Start,
           T Stop,
           T... Indexes,
           class = std::enable_if_t<std::greater<T>{}(Stop, Start)> >
constexpr auto make_range_impl(std::integer_sequence<T, Indexes...>)
{
    return std::integer_sequence<T, (Indexes + Start)...>{};
}

template < typename T,
           T Start,
           T Stop,
           T... Indexes >
constexpr auto make_range_impl(std::integer_sequence<T, Indexes...>)
{
    return std::integer_sequence<T, (Start - Indexes)...>{};
}

template < typename T,
           T Start,
           T Stop >
constexpr auto make_range(std::integral_constant<T, Start> from,
                          std::integral_constant<T, Stop>  to)
{
    static_assert(Start != Stop, "Index difference must be greater than zero");

    if constexpr (Start < Stop)
    {
        return make_range_impl<T, Start, Stop>(std::make_integer_sequence<T, Stop - Start>{});
    }
    else
    {
        return make_range_impl<T, Start, Stop>(std::make_integer_sequence<T, Start - Stop>{});
    }
}

template<std::size_t Index>
constexpr auto num = std::integral_constant<std::size_t, Index>{};

template<typename... Args, std::size_t... Indexes>
constexpr auto _first_impl(std::tuple<Args...> t, std::index_sequence<Indexes...>)
{
    return std::make_tuple(std::get<Indexes>(t)...);
}

template<typename... Args, std::size_t... Indexes>
constexpr auto _last_impl(std::tuple<Args...> t, std::index_sequence<Indexes...>)
{
    return std::make_tuple(std::get<sizeof...(Args) - Indexes>(t)...);
}

template<std::size_t N, typename... Args>
constexpr auto take_first_n(std::tuple<Args...> t)
{
    static_assert(N > 0, "Taken elements count must be greater than zero");
    static_assert(N <= sizeof...(Args), "Taken elements count must be less or equal than std::tuple size");
    return _first_impl(t, std::make_index_sequence<N>{});
}

template<std::size_t N, typename... Args>
constexpr auto take_last_n(std::tuple<Args...> t)
{
    static_assert(N > 0, "Taken elements count must be greater than zero");
    static_assert(N <= sizeof...(Args), "Taken elements count must be less or equal than std::tuple size");
    return _last_impl(t, make_range(num<N>, num<0>));
}

template<typename Functor, typename... Args>
constexpr auto curry(Functor&& f, Args&&... args)
{
    return f(std::tuple<Args...> { std::forward<Args...>(args)... });
}

template<typename Functor, typename... Args>
constexpr auto uncurry(Functor&& f, std::tuple<Args...> t)
{
    return std::apply(std::forward<Functor>(f), t);
}

template<typename... Types, typename Func, std::size_t... I>
constexpr void tuple_visit_impl(std::tuple<Types...> t, Func&& func, std::index_sequence<I...>)
{
    ((func(std::get<I>(t))), ...);
}

template<typename... Types, typename Func>
constexpr void tuple_visit(std::tuple<Types...> t, Func&& f)
{
    tuple_visit_impl(t,
                     std::forward<Func>(f),
                     std::make_index_sequence<sizeof...(Types)>{});
}

template<typename... Types, typename Func, std::size_t... I>
constexpr auto tuple_transform_impl(std::tuple<Types...> t, Func&& func, std::index_sequence<I...>)
{
    return std::make_tuple(func(std::get<I>(t))...);
}

template<typename... Types, typename Func>
constexpr auto tuple_transform(std::tuple<Types...> t, Func&& f)
{
    return tuple_transform_impl(t,
                                std::forward<Func>(f),
                                std::make_index_sequence<sizeof...(Types)>{});
}

template <std::size_t Count, std::size_t Current, typename... Input, typename Initial, typename Func>
constexpr auto tuple_fold_impl(std::tuple<Input...> t, Initial&& initial, Func&& functor)
{
    if constexpr (Current < Count)
    {
        initial = functor(std::forward<Initial>(initial), std::get<Current>(t));
        return tuple_fold_impl<Count, Current + 1>
            (t, std::forward<Initial>(initial), std::forward<Func>(functor));
    }
    else
    {
        return initial;
    }
}

template <typename... Input, typename Initial, typename Func>
constexpr auto tuple_fold(std::tuple<Input...> t, Initial&& initial, Func&& functor)
{
    return tuple_fold_impl<sizeof...(Input), 0>(t,
                                                std::forward<Initial>(initial),
                                                std::forward<Func>(functor));
}

}

#endif //CAMERAMANAGERCORE_TUPLEUTILITY_HPP
