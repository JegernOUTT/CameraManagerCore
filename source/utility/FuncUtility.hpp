//
// Created by svakhreev on 20.03.17.
//

#ifndef CAMERAMANAGERCORE_FUNCUTILITY_HPP
#define CAMERAMANAGERCORE_FUNCUTILITY_HPP

#include <numeric>
#include <functional>
#include <type_traits>

namespace cameramanagercore::utility
{

template <typename Sig, typename = void>
struct is_invocable : std::false_type { };

template <typename Func, typename... Args>
struct is_invocable < Func(Args...),
                      std::void_t<decltype(std::declval<Func>()(std::declval<Args&>()...))> >
    : std::true_type
{
    using type = decltype(std::declval<Func>()(std::declval<Args&>()...));
};

template<typename Func, typename... Args>
using is_invocable_t = typename is_invocable<Func, Args...>::type;

template<typename Func, typename... Args>
constexpr bool is_invocable_v = is_invocable<Func, Args...>::value;

struct func_ubiq
{
    std::size_t ignore;
    template<typename T> constexpr operator T&() const noexcept;
};

template <typename Func, typename... ExtraArgs>
struct _return_type_impl
{
private:
    template<std::size_t... Indexes>
    constexpr static auto get_return_type(std::index_sequence<Indexes...>)
    {
        static_assert(sizeof...(Indexes) < 15, "Too many parameters");

        if constexpr (is_invocable_v<Func(ExtraArgs..., decltype(func_ubiq{Indexes})...)>)
        {
            return is_invocable_t<Func(ExtraArgs..., decltype(func_ubiq{Indexes})...)> {};
        }
        else
        {
            return get_return_type(std::make_index_sequence<sizeof...(Indexes) + 1>{});
        }
    }

public:
    using type = decltype(get_return_type(std::make_index_sequence<0>{}));
};

template<typename Func, typename... ExtraArgs>
struct return_type
{
    using type = std::decay_t<typename _return_type_impl<std::decay_t<Func>, ExtraArgs...>::type>;
};

template<typename Func, typename... ExtraArgs>
using return_type_t = typename return_type<Func, ExtraArgs...>::type;

}

#endif //CAMERAMANAGERCORE_FUNCUTILITY_HPP
