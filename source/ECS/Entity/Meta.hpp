namespace Meta
{
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
    std::vector<T> makeVector(const std::array<T, N> pArray )
    {
        return std::vector<T>(pArray.begin(), pArray.end());
    }

    // Returns the sum of the sizof of each individual argument in the variadic.
    template <typename... Args>
    constexpr static size_t sizeOfVariadic()
    {
        return (0 + ... + sizeof(Args));
    }
}