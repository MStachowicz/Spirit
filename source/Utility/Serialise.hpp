#pragma once

#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace Utility
{
	class BinaryWriter;
	class BinaryReader;

	// Raw values use native layout and byte order; this is not a portable file format.
	template <typename T>
	concept Is_Trivially_Serializable = std::is_standard_layout_v<T> && std::is_trivial_v<T>
		&& !std::is_pointer_v<T> && !std::is_member_pointer_v<T>;

	template <typename T>
	concept Has_Member_Func_Custom_Serialisation = requires(BinaryWriter& p_out, BinaryReader& p_in, const T& p_const_value, T& p_value)
	{
		{ p_const_value.write_binary(p_out) } -> std::same_as<void>;
		{ p_value.read_binary(p_in) } -> std::same_as<void>;
	};

	template <typename T>
	concept Has_Static_Func_Custom_Serialisation = requires(BinaryWriter& p_out, BinaryReader& p_in, const T& p_value)
	{
		{ T::serialise(p_out, p_value) } -> std::same_as<void>;
		{ T::deserialise(p_in) } -> std::same_as<T>;
	};

	template <typename T>
	concept Has_Custom_Serialisation = Has_Member_Func_Custom_Serialisation<T> || Has_Static_Func_Custom_Serialisation<T>;

	template <typename T>
	concept Is_POD_And_Not_Custom_serialisable = !Has_Custom_Serialisation<T> && Is_Trivially_Serializable<T>;

	template <typename T>
	concept Is_Container = requires(T p_container, typename T::value_type p_value)
	{
		{ p_container.size() } -> std::convertible_to<std::size_t>;
		{ p_container.begin() } -> std::input_or_output_iterator;
		{ p_container.end() } -> std::input_or_output_iterator;
		p_container.push_back(std::move(p_value));
	};

	template <typename T>
	concept Is_Contiguous_Container = Is_Container<T> && requires(T p_container)
	{
		{ p_container.begin() } -> std::contiguous_iterator;
		p_container.data();
		p_container.resize(std::size_t{});
	};

	template <typename T>
	struct Is_Serializable
	{
		static constexpr bool value = Is_Trivially_Serializable<T> || Has_Static_Func_Custom_Serialisation<T>
			|| (Has_Member_Func_Custom_Serialisation<T> && std::is_default_constructible_v<T>);
	};

	template <Is_Container T>
	struct Is_Serializable<T>
	{
		static constexpr bool value = Has_Custom_Serialisation<T> || Is_Serializable<typename T::value_type>::value;
	};

	template <typename T>
	inline constexpr bool Is_Serializable_v = Is_Serializable<T>::value;

	class BinaryWriter
	{
	public:
		explicit BinaryWriter(std::vector<std::byte>& p_destination) noexcept
			: m_destination{p_destination}
		{}

		// The source must not alias the destination vector.
		void write_bytes(std::span<const std::byte> p_bytes)
		{
			m_destination.insert(m_destination.end(), p_bytes.begin(), p_bytes.end());
		}

		std::size_t position() const noexcept { return m_destination.size(); }

		template <typename T>
		void write(const T& p_value)
		{
			static_assert(Is_Serializable_v<T>, "Provide custom serialisation for this type.");

			if constexpr (Has_Static_Func_Custom_Serialisation<T>)
				T::serialise(*this, p_value);
			else if constexpr (Has_Member_Func_Custom_Serialisation<T>)
				p_value.write_binary(*this);
			else if constexpr (Is_Container<T>)
			{
				using Value = typename T::value_type;
				write(static_cast<std::size_t>(p_value.size()));
				if constexpr (Is_Contiguous_Container<T> && Is_POD_And_Not_Custom_serialisable<Value>)
					write_bytes(std::as_bytes(std::span{p_value.data(), p_value.size()}));
				else
					for (const auto& element : p_value)
						write<Value>(element);
			}
			else
				write_bytes(std::as_bytes(std::span<const T>{&p_value, 1}));
		}

	private:
		std::vector<std::byte>& m_destination;
	};

	class BinaryReader
	{
	public:
		explicit BinaryReader(std::span<const std::byte> p_source) noexcept
			: m_source{p_source}
		{}

		void read_bytes(std::span<std::byte> p_destination)
		{
			if (p_destination.size() > remaining())
				throw std::runtime_error("Truncated binary data.");
			if (!p_destination.empty())
				std::memcpy(p_destination.data(), m_source.data() + m_cursor, p_destination.size());
			m_cursor += p_destination.size();
		}

		std::size_t position() const noexcept { return m_cursor; }
		std::size_t remaining() const noexcept { return m_source.size() - m_cursor; }

		template <typename T>
		T read()
		{
			static_assert(Is_Serializable_v<T>, "Provide custom deserialisation for this type.");

			if constexpr (Has_Static_Func_Custom_Serialisation<T>)
				return T::deserialise(*this);
			else if constexpr (Has_Member_Func_Custom_Serialisation<T>)
			{
				T value;
				value.read_binary(*this);
				return value;
			}
			else if constexpr (Is_Container<T>)
			{
				using Value = typename T::value_type;
				const auto count = read<std::size_t>();
				T value;
				if (count > value.max_size())
					throw std::length_error("Binary container size exceeds max_size.");

				if constexpr (Is_Contiguous_Container<T> && Is_POD_And_Not_Custom_serialisable<Value>)
				{
					if (count > remaining() / sizeof(Value))
						throw std::runtime_error("Truncated binary container.");
					value.resize(count);
					read_bytes(std::as_writable_bytes(std::span{value.data(), value.size()}));
				}
				else
				{
					if constexpr (requires { value.reserve(count); })
						value.reserve(count);
					for (std::size_t i = 0; i < count; ++i)
						value.push_back(read<Value>());
				}
				return value;
			}
			else
			{
				std::array<std::byte, sizeof(T)> bytes;
				read_bytes(bytes);
				return std::bit_cast<T>(bytes);
			}
		}

		template <typename T>
		void read(T& p_destination)
		{
			p_destination = read<T>();
		}

	private:
		std::span<const std::byte> m_source;
		std::size_t m_cursor = 0;
	};
} // namespace Utility
