#pragma once

#include <stddef.h>

namespace Utility
{
    namespace Impl
    {
        template <typename... Args>
        struct ArgTypes
        {};

        template <typename T, typename... Args>
        struct ArgTypes<T, Args...>
        {
            using Head = T;
            using Tail = ArgTypes<Args...>;
        };

        template <typename... Args>
        using ExtractArgTypes = ArgTypes<Args...>;
    }

    template <typename Func>
    struct FunctionTraits : public FunctionTraits<decltype(&Func::operator())>
    {};

    template <typename ClassType, typename ReturnType, typename... Args>
    struct FunctionTraits<ReturnType (ClassType::*)(Args...) const>
    {
        using Return                         = ReturnType;
        using ArgsTuple                      = std::tuple<Args...>;
        using ArgTypes                       = Impl::ExtractArgTypes<Args...>;
        static constexpr std::size_t NumArgs = std::tuple_size_v<ArgsTuple>;
    };

    template <typename ReturnType>
    struct FunctionTraits<ReturnType()>
    {
        using Return                         = ReturnType;
        using ArgsTuple                      = std::tuple<>;
        using ArgTypes                       = Impl::ExtractArgTypes<>;
        static constexpr std::size_t NumArgs = 0;
    };

    template <typename Func>
    using ReturnType = typename FunctionTraits<Func>::Return;

    template <typename Func>
    using ArgsTuple = typename FunctionTraits<Func>::ArgsTuple;

    template <typename Func, size_t N>
    using ArgTypeN = typename std::tuple_element_t<N, ArgsTuple<Func>>;
} // namespace Utility