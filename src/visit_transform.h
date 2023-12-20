#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

template <class T>
auto forward_as(std::add_lvalue_reference_t<std::remove_reference_t<T>> u)
    -> std::conditional_t<std::is_rvalue_reference_v<T>, std::remove_reference_t<T>, T>
{
    if constexpr (std::is_rvalue_reference_v<T> || !std::is_reference_v<T>)
    {
        return std::move(u);
    }
    else
    {
        return u;
    }
}

namespace detail
{
// template <size_t N>
// struct TT
// {
//     template <class Function, class Transformer, class T, class... Args>
//     constexpr decltype(auto) operator()(Function&& function, Transformer&& transformer, T&& t, Args&&... args) const
//     {
//         if constexpr (N > 1)
//         {
//             return transformer(
//                 [&]<class U>(U&& transformed_t) -> decltype(auto)
//                 {
//                     return TT<N - 1>{}(function, transformer, std::forward<Args>(args)...,
//                                        std::forward<U>(transformed_t));
//                 },
//                 forward_as<T>(t));
//         }
//         else
//         {
//             return transformer(
//                 [&]<class U>(U&& transformed_t) -> decltype(auto)
//                 {
//                     return function(std::forward<Args>(args)..., std::forward<U>(transformed_t));
//                 },
//                 forward_as<T>(t));
//         }
//     }
// };

template <class Function, class Transformer, class T, class... Args>
constexpr decltype(auto) visit_transform2(std::integral_constant<size_t, 1>, Function&& function,
                                          Transformer&& transformer, T&& t, Args&&... args)
{
    return transformer(
        [&]<class U>(U&& transformed_t) -> decltype(auto)
        {
            return function(std::forward<Args>(args)..., std::forward<U>(transformed_t));
        },
        forward_as<T>(t));
}

template <size_t N, class Function, class Transformer, class T, class... Args>
constexpr decltype(auto) visit_transform2(std::integral_constant<size_t, N>, Function&& function,
                                          Transformer&& transformer, T&& t, Args&&... args)
{
    return transformer(
        [&]<class U>(U&& transformed_t) -> decltype(auto)
        {
            return detail::visit_transform2(std::integral_constant<size_t, N - 1>{}, function, transformer,
                                            std::forward<Args>(args)..., std::forward<U>(transformed_t));
        },
        forward_as<T>(t));
}
}  // namespace detail

template <class Function, class Transformer, class... T>
constexpr decltype(auto) visit_transform2(Function&& function, Transformer&& transformer, T&&... t)
{
    return detail::visit_transform2(std::integral_constant<size_t, sizeof...(T)>{}, function, transformer,
                                    std::forward<T>(t)...);
}

namespace detail
{
template <class T, class Transformer, class Function>
struct TransformVisitor
{
    using TRef = std::add_lvalue_reference_t<std::remove_reference_t<T>>;

    TRef t;
    Transformer transformer;
    Function function;

    template <class... Args>
    constexpr decltype(auto) operator()(Args&&... args)
    {
        return transformer(
            [&]<class U>(U&& transformed_t) -> decltype(auto)
            {
                return function(std::forward<Args>(args)..., std::forward<U>(transformed_t));
            },
            forward_as<T>(t));
    }

    template <class OtherT, class OtherFunction>
    friend constexpr auto operator|(TransformVisitor lhs,
                                    detail::TransformVisitor<OtherT, Transformer, OtherFunction> rhs) noexcept
    {
        return detail::TransformVisitor<T, Transformer, detail::TransformVisitor<OtherT, Transformer, OtherFunction>>{
            lhs.t, lhs.transformer, {rhs.t, rhs.transformer, rhs.function}};
    }
};
}  // namespace detail

template <class Function, class Transformer, class... T>
constexpr decltype(auto) visit_transform(Function&& function, Transformer&& transformer, T&&... t)
{
    return (detail::TransformVisitor<T&&, Transformer&, Function&>{t, transformer, function} | ...)();
}
