// How to determine what type of arguments a function that takes a function
// passes to its invocation.
// The pattern is known as friend injection.

// Here we want to determine that the function `production_code` invokes F with an `int`.

#include <cstddef>
#include <functional>
#include <variant>

//
#include <cassert>
#include <string>
#include <string_view>

namespace meta
{
namespace detail
{
// template <class T>
// struct Unitialized
// {
//     Unitialized() {}

//     ~Unitialized() noexcept {}

//     union
//     {
//         T value;
//     };
// };

// template <class Variant, size_t Count, size_t Size, class Tag>
// requires(MAX_SIZE<Variant> <= Size) auto unwrap(Erased<Count, Size, Tag>& e)
// {
//     Unitialized<Variant> variant;
//     [&]<size_t... I>(std::index_sequence<I...>)
//     {
//         (
//             [&]
//             {
//                 if (e.index == I)
//                 {
//                     using Type = std::variant_alternative_t<I, Variant>;
//                     std::construct_at(&variant.value, std::in_place_index<I>,
//                                       std::move(*reinterpret_cast<Type*>(e.value)));
//                 }
//             }(),
//             ...);
//     }
//     (std::make_index_sequence<std::variant_size_v<Variant>>{});
//     return std::move(variant.value);
// }

template <class List, class T>
struct AppendTypeToList;

template <template <class...> class List, class T, class... U>
struct AppendTypeToList<List<U...>, T>
{
    using Type = List<U..., T>;
};

template <class List, class T>
using AppendTypeToListT = typename AppendTypeToList<List, T>::Type;

template <class It>
constexpr It max_element_unchecked(It first, It last)
{
    It found = first;
    if (first != last)
    {
        while (++first != last)
        {
            if (*found < *first)
            {
                found = first;
            }
        }
    }
    return found;
}

template <class T>
constexpr T max_value(std::initializer_list<T> list)
{
    const T* result = detail::max_element_unchecked(list.begin(), list.end());
    return *result;
}

template <class Variant>
inline constexpr auto MAX_SIZE = 0;

template <template <class...> class List, class... T>
inline constexpr auto MAX_SIZE<List<T...>> = detail::max_value({sizeof(T)...});

// Part of C++23
[[noreturn]] inline void unreachable()
{
#ifdef __GNUC__  // GCC, Clang, ICC
    __builtin_unreachable();
#elif defined(_MSC_VER)  // MSVC
    __assume(false);
#endif
}

namespace flag_ns
{
template <std::size_t Index, class Tag>
struct Flag
{
    friend constexpr auto flag(const Flag&);
};

template <class T, std::size_t Index, class Tag>
struct FlagInjector
{
    static constexpr auto INDEX = Index;

    friend constexpr auto flag(const Flag<Index, Tag>&)
    {
        if constexpr (Index > 0)
        {
            using Previous = typename decltype(flag(Flag<Index - 1, Tag>{}))::type;
            using Next = detail::AppendTypeToListT<Previous, T>;
            return std::type_identity<Next>{};
        }
        else
        {
            return std::type_identity<std::variant<T>>{};
        }
    }
};

template <class T, std::size_t Index, class Tag>
constexpr auto instantiate_next_flag(long)
{
    return FlagInjector<T, Index, Tag>{};
}

template <class T, std::size_t Index, class Tag, auto = flag(Flag<Index, Tag>{})>
constexpr auto instantiate_next_flag(int)
{
    return flag_ns::instantiate_next_flag<T, Index + 1, Tag>(int{});
}

template <std::size_t Index, class Tag>
concept ValidFlag = requires(const Flag<Index, Tag>& v)
{
    flag(v);
};

template <class FlagT>
using FlagInvokeResult = decltype(flag(std::declval<const FlagT&>()));
}

using flag_ns::Flag;
using flag_ns::FlagInvokeResult;
using flag_ns::instantiate_next_flag;
using flag_ns::ValidFlag;

template <std::size_t Size, class TagT>
struct Erased
{
    using Tag = TagT;

    std::size_t index;
    alignas(alignof(std::max_align_t)) std::byte value[Size];
};

template <std::size_t I, class Variant, size_t Size, class Tag>
auto to_variant_recursive(Erased<Size, Tag>& e)
{
    if (e.index == I)
    {
        return Variant{std::in_place_index<I>,
                       std::move(*reinterpret_cast<std::variant_alternative_t<I, Variant>*>(e.value))};
    }
    if constexpr (I < std::variant_size_v<Variant> - 1)
    {
        return detail::to_variant_recursive<I + 1, Variant>(e);
    }
    else
    {
        detail::unreachable();
    }
}

template <class Variant, std::size_t Size, class Tag>
requires(MAX_SIZE<Variant> <= Size) constexpr auto to_variant(Erased<Size, Tag>& e)
{
    return detail::to_variant_recursive<0, Variant>(e);
}

template <std::size_t I, class Tag, bool = detail::ValidFlag<I, Tag>, bool = detail::ValidFlag<I + 1, Tag>>
struct LastFlagInstantation : LastFlagInstantation<I + 1, Tag>
{
};

template <std::size_t I, class Tag>
struct LastFlagInstantation<I, Tag, true, false>
{
    using Type = Flag<I, Tag>;
};

template <size_t Size, class Tag>
struct VariateFn
{
    template <class Arg, auto Injector = detail::instantiate_next_flag<std::decay_t<Arg>, 0, Tag>(int{})>
    constexpr auto operator()(Arg&& arg) const
    {
        Erased<Size, Tag> e;
        e.index = Injector.INDEX;
        std::construct_at(reinterpret_cast<std::decay_t<Arg>*>(e.value), std::forward<Arg>(arg));
        return e;
    }
};
}

template <size_t Size = 256, auto Tag = [] {}>
inline constexpr detail::VariateFn<Size, decltype(Tag)> variate{};

template <class F, class... Args>
// GCC decides to out-line this function when `Size` above gets too big,
// although after inlining there is zero code size increase.
#ifdef __GNUC__  // GCC, Clang, ICC
__attribute__((always_inline)) inline
#elif defined(_MSC_VER)  // MSVC
__forceinline
#endif
    auto
    make_variant(F&& f, Args&&... args)
{
    auto erased = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    using Erased = decltype(erased);
    using Flag = typename detail::LastFlagInstantation<0, typename Erased::Tag>::Type;
    using Variant = typename detail::FlagInvokeResult<Flag>::type;
    return detail::to_variant<Variant>(erased);
}
}

namespace prod
{
static auto code(bool ok)
{
    static constexpr auto var = meta::variate<>;
    if (ok)
    {
        return var(42);
    }
    return var(std::string("a very very long test test"));
}

static auto code1()
{
    static constexpr auto var = meta::variate<>;
    return var(42);
}

static auto code2(bool ok)
{
    static constexpr auto var = meta::variate<>;
    if (ok)
    {
        return var(1.5f);
    }
    return var("a very very long test test");
}

static auto code3(int ok)
{
    static constexpr auto var = meta::variate<>;
    if (ok <= 5)
    {
        return var(1.5f);
    }
    if (ok > 5 && ok <= 10)
    {
        return var(42);
    }
    return var(std::string("a very very long test test"));
}

static auto comp3(int ok) -> std::variant<float, int, std::string>
{
    if (ok <= 5)
    {
        return 1.5f;
    }
    if (ok > 5 && ok <= 10)
    {
        return 42;
    }
    return std::string("a very very long test test");
}
}

auto a(bool ok)
{
    auto r = meta::make_variant(prod::code2, ok);
    static_assert(std::is_same_v<decltype(r), std::variant<float, const char*>>);
    if (ok)
    {
        assert(1.5f == std::get<0>(r));
    }
    else
    {
        assert("a very very long test test" == std::string_view(std::get<1>(r)));
    }
    return r;
}

auto b()
{
    auto r = meta::make_variant(prod::code1);
    static_assert(std::is_same_v<decltype(r), std::variant<int>>);
    assert(42 == std::get<0>(r));
    return r;
}

int main()
{
    a(true);
    a(false);
    auto v = meta::make_variant(prod::code, true);
    static_assert(std::is_same_v<decltype(v), std::variant<int, std::string>>);
    assert(42 == std::get<0>(v));
    assert(std::string("a very very long test test") == std::get<2>(meta::make_variant(prod::code3, 12)));
}