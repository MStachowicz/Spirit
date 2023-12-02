#pragma once

#include <tuple>
#include <vector>
#include <algorithm>

namespace Meta
{
    // By leveraging type deduction, PackArg can be used to supply a type only parameter to a function.
    // Useful for use as a parameter to a constructor which could otherwise not be a template only function.
    template <typename Arg>
    struct PackArg
    {};

    // Encapsulates a parameter pack
    // Acts as a container to transfer the parameter types of the function.
    template<typename... Args>
    struct PackArgs
    {};

    /* Get the FunctionInformation of a callable/function
    Usage for a normal function
        using arg_types_pack = typename GetFunctionInformation<decltype(&func)>::GetParameterPack;
    Usage for a lambda
        using arg_types_pack = typename GetFunctionInformation<Func>::GetParameterPack; */
    template <typename Func, typename = void, typename = void>
    struct GetFunctionInformation;

    template <typename Return, typename... Args>
    struct GetFunctionInformation<Return(*)(Args...)>
    {
        using GetParameterPack = PackArgs<Args...>;
    };

    template <typename Return, typename Class, typename... Args>
    struct GetFunctionInformation<Return(Class::*)(Args...)>
    {
        using GetParameterPack = PackArgs<Args...>;
    };

    template <typename Return, typename Class, typename... Args>
    struct GetFunctionInformation<Return(Class::*)(Args...) const>
    {
        using GetParameterPack = PackArgs<Args...>;
    };

    template <typename T>
    struct GetFunctionInformation<T, std::void_t<decltype(&T::operator())>>
    : public GetFunctionInformation<decltype(&T::operator())>
    {};

    // Helpers to deduce if all the params are unique types.
    template <typename...>
    inline constexpr auto is_unique = std::true_type{};
    template <typename T, typename... Rest>
    inline constexpr auto is_unique<T, Rest...> = std::bool_constant<(!std::is_same_v<T, Rest> && ...) && is_unique<Rest...>>{};

    // Convert a std::array to a std::vector in one call.
    template<typename T, size_t N>
    std::vector<T> make_vector(const std::array<T, N> p_array )
    {
        return std::vector<T>(p_array.begin(), p_array.end());
    }

    // Returns the size of a parameter pack in bytes.
    // Sums each individual argument in the variadic.
    template <typename... Args>
    constexpr static size_t sizeOfVariadic()
    {
        return (0 + ... + sizeof(Args));
    }

    // Returns the max of all the alignof parameter pack types.
    template <typename... Args>
    constexpr static inline size_t get_max_alignof()
    {
        return (std::max({alignof(std::decay_t<Args>)...}));
    }

    template<std::size_t N, typename... Args>
    struct GetNth
    {
        using Type = std::tuple_element_t<N, std::tuple<Args...>>;
    };

    // Does Type appear as one of the Args.
    template <typename Type, typename... Args>
    static constexpr bool hasType()
    { // Fold on || returns false by default so this function also works with 0 Args.
        return (std::is_same_v<Type, std::decay_t<Args>> || ...);
    }
}