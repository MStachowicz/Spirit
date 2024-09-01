#pragma once

#include <concepts>
#include <iostream>
#include <type_traits>
#include <stdint.h>

// Helpers for serialising and deserialising objects.
// write_binary and read_binary are used to write and read objects to and from a binary stream.
// Supports plain old data types, custom serialisable types, and containers of serialisable types.
// Whether a type is serialisable is determined by the Is_Serializable concept.
namespace Utility
{
	// For plain old data types, we can just write the data directly to the stream.
	template <typename T>
	concept Is_Trivially_Serializable = (std::is_standard_layout_v<T> && std::is_trivial_v<T>);

	// Custom serialisation can be achieved by defining a write_binary and read_binary member function.
	template <typename T>
	concept Has_Member_Func_Custom_Serialisation = requires(std::ostream& p_out, std::istream& p_in, uint16_t p_version, T& p_value)
	{
		{ p_value.write_binary(p_out, p_version) };
		{ p_value.read_binary(p_in, p_version)   };
	};

	// Custom serialisation can also be achieved by defining static serialise and deserialise functions.
	template <typename T>
	concept Has_Static_Func_Custom_Serialisation = requires(std::ostream& p_out, std::istream& p_in, uint16_t p_version, const T& p_const_value, T& p_value)
	{
		{ T::serialise(p_out, p_version, p_const_value) };
		{ T::deserialise(p_in, p_version) } -> std::same_as<T>;
	};

	template <typename T>
	concept Has_Custom_Serialisation = Has_Member_Func_Custom_Serialisation<T> || Has_Static_Func_Custom_Serialisation<T>;

	template <typename T>
	concept Is_POD_And_Not_Custom_serialisable = !Has_Custom_Serialisation<T> && Is_Trivially_Serializable<T>;

	template <typename T>
	concept Is_Container = requires(T container)
	{
		typename T::value_type;     // Must have a value_type
		typename T::iterator;       // Must have an iterator
		typename T::const_iterator; // Must have a const_iterator

		{ container.size() }  -> std::convertible_to<std::size_t>;  // Must have a size() member function
		{ container.begin() } -> std::input_or_output_iterator;     // begin() must return an input or output iterator
		{ container.end() }   -> std::input_or_output_iterator;     // end() must return an input or output iterator
	};

	template <typename T>
	concept Is_Contiguous_Container = requires(T container)
	{
		typename T::value_type;    // Must have a value_type
		typename T::pointer;       // Must have a pointer type
		typename T::const_pointer; // Must have a const_pointer type

		{ container.size() }  -> std::convertible_to<std::size_t>;  // Must have a size() member function
		{ container.begin() } -> std::contiguous_iterator;          // begin() must return a contiguous iterator
		{ container.end() }   -> std::contiguous_iterator;          // end() must return a contiguous iterator
		{ container.data() }  -> std::same_as<typename T::pointer>; // Must have a data() member function returning a pointer
	};

	// Trait to determine if a type is serialisable.
	template <typename T>
	struct Is_Serializable { static constexpr bool value = Is_Trivially_Serializable<T> || Has_Custom_Serialisation<T>; };

	// Trait to determine if a container is serialisable.
	template <Is_Container T>
	struct Is_Serializable<T> { static constexpr bool value = Is_Serializable<typename T::value_type>::value; };

	// Helper variable template to determine if a type is serialisable.
	template <typename T>
	inline constexpr bool Is_Serializable_v = Is_Serializable<T>::value;

	// Write a custom serialisable object to a binary stream.
	template<Has_Custom_Serialisation T>
	inline void write_binary(std::ostream& p_out, uint16_t p_version, const T& p_value)
	{
		if constexpr (Has_Member_Func_Custom_Serialisation<T>)
			p_value.write_binary(p_out, p_version);
		else if constexpr (Has_Static_Func_Custom_Serialisation<T>)
			T::serialise(p_out, p_version, p_value);
	}
	// Read a custom serialisable object from a binary stream.
	template<Has_Custom_Serialisation T>
	inline void read_binary(std::istream& p_in, uint16_t p_version, T& p_value)
	{
		if constexpr (Has_Member_Func_Custom_Serialisation<T>)
			p_value.read_binary(p_in, p_version);
		else if constexpr (Has_Static_Func_Custom_Serialisation<T>)
			p_value = T::deserialise(p_in, p_version);
	}

	// Write a POD type to a binary stream.
	template<Is_POD_And_Not_Custom_serialisable T>
	inline void write_binary(std::ostream& p_out, uint16_t p_version, const T& p_value)
	{ (void)p_version;
		p_out.write(reinterpret_cast<const char*>(&p_value), sizeof(p_value));
	}
	// Read a POD type from a binary stream.
	template<Is_POD_And_Not_Custom_serialisable T>
	inline void read_binary(std::istream& p_in, uint16_t p_version, T& p_value)
	{ (void)p_version;
		p_in.read(reinterpret_cast<char*>(&p_value), sizeof(p_value));
	}

	// Write a container to a binary stream.
	// Recursively writes each element of the container.
	template<Is_Container T>
	inline void write_binary(std::ostream& p_out, uint16_t p_version, const T& p_container)
	{
		using value_type = typename T::value_type;
		static_assert(Is_Serializable_v<value_type>, "Container value_type must be serialisable. Provide a custom serialisation function if necessary.");

		// If we are writing a contiguous container, we can write the size and then the data in one go.
		// However in the case the container::value_type is not trivially serialisable, we need to serialise each element individually.

		std::size_t num_of_elements = p_container.size();
		p_out.write(reinterpret_cast<const char*>(&num_of_elements), sizeof(num_of_elements));

		if constexpr (Is_Trivially_Serializable<value_type> && Is_Contiguous_Container<T>)
		{
			p_out.write(reinterpret_cast<const char*>(p_container.data()), num_of_elements * sizeof(value_type));
		}
		else
		{
			for (const auto& element : p_container)
				write_binary(p_out, p_version, element);
		}
	}

	// Read a container from a binary stream.
	// Recursively reads each element of the container.
	template<Is_Container T>
	inline void read_binary(std::istream& p_in, uint16_t p_version, T& p_container)
	{
		using value_type = typename T::value_type;
		static_assert(Is_Serializable_v<value_type>, "Container value_type must be serialisable. Provide a custom serialisation function if necessary.");

		// Read the size of the container and then read the data in one go.
		// If the container::value_type is not trivially serialisable, we need to deserialise each element individually.

		std::size_t num_of_elements;
		p_in.read(reinterpret_cast<char*>(&num_of_elements), sizeof(num_of_elements));
		p_container.resize(num_of_elements);

		if constexpr (Is_Trivially_Serializable<value_type> && Is_Contiguous_Container<T>)
		{
			p_in.read(reinterpret_cast<char*>(p_container.data()), num_of_elements * sizeof(value_type));
		}
		else
		{
			for (auto& element : p_container)
				read_binary(p_in, p_version, element);
		}
	}
} // namespace Utility