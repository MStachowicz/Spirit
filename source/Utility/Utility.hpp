#pragma once

namespace util
{
    template <typename T>
    static constexpr auto toIndex(const T& pEnum) noexcept // Returns the underlying type. Used to convert enum types to indexes into arrays
    {
        return static_cast<std::underlying_type_t<T>>(pEnum);
    }
}