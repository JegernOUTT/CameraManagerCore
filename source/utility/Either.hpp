
#ifndef EITHER_EITHER_HPP
#define EITHER_EITHER_HPP

#include <variant>
#include <optional>

template < typename F, typename Arg >
concept bool invokable_with = requires(F f, Arg arg) { f(arg); };

template < typename left_type_, typename right_type_ >
struct either
{
    using left_type = left_type_;
    using right_type = right_type_;

    constexpr either(const left_type& lt) noexcept
        : _impl(lt)
    {}

    constexpr either(left_type&& lt) noexcept
        : _impl(std::move(lt))
    {}

    constexpr either(const right_type& rt) noexcept
        : _impl(rt)
    {}

    constexpr either(right_type&& rt) noexcept
        : _impl(std::move(rt))
    {}

    constexpr either& operator=(const left_type& lt) noexcept
    {
        _impl = lt;
        return *this;
    }

    constexpr either& operator=(const left_type&& lt) noexcept
    {
        _impl = std::move(lt);
        return *this;
    }

    constexpr either& operator=(const right_type& rt) noexcept
    {
        _impl = rt;
        return *this;
    }

    constexpr either& operator=(const right_type&& rt) noexcept
    {
        _impl = std::move(rt);
        return *this;
    }

    constexpr bool is_left() const noexcept
    {
        return std::get_if<left_type>(&_impl) != 0;
    }

    constexpr bool is_right() const noexcept
    {
        return !is_left();
    }

    constexpr std::optional<left_type> maybe_left() const noexcept
    {
        return is_left() ? std::get<left_type>(_impl)
                         : std::optional<left_type>{};
    }

    constexpr std::optional<right_type> maybe_right() const noexcept
    {
        return !is_left() ? std::get<right_type>(_impl)
                          : std::optional<right_type>{};
    }

    constexpr const left_type& left() const
    {
        return std::get<left_type>(_impl);
    }

    constexpr const right_type& right() const
    {
        return std::get<right_type>(_impl);
    }

    constexpr const either& if_left_do(const invokable_with<left_type>&& f) const
    {
        if (is_left()) f(std::get<left_type>(_impl));
        return *this;
    }

    constexpr const either& if_right_do(const invokable_with<right_type>&& f) const
    {
        if (is_right()) f(std::get<right_type>(_impl));
        return *this;
    }

    template <typename Functor>
    requires invokable_with<Functor, right_type> || invokable_with<Functor, left_type>
    constexpr either& match_call(Functor&& f)
    {
        if constexpr (invokable_with<Functor, right_type>)
        {
            if (is_right()) f(std::get<right_type>(_impl));
        }
        if constexpr (invokable_with<Functor, left_type>)
        {
            if (is_left()) f(std::get<left_type>(_impl));
        }
        return *this;
    }

private:
    std::variant<left_type, right_type> _impl;
};

namespace std
{
template < typename get_type, typename left_type, typename right_type >
constexpr auto get(const either<left_type, right_type>& et)
{
    if constexpr (std::is_same_v<get_type, right_type>)
    {
        return et.right();
    }
    else
    {
        return et.left();
    }
};

template < typename get_type, typename left_type, typename right_type >
constexpr auto get(either<left_type, right_type>&& et)
{
    if constexpr (std::is_same_v<get_type, right_type>)
    {
        return et.right();
    }
    else
    {
        return et.left();
    }
};
}

#endif //EITHER_EITHER_HPP