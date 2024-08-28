#pragma once

#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include <concepts>
#include <list>

// Helpers for serialising and deserialising objects.
namespace Utility
{
	// For plain old data types, we can just write the data directly to the stream.
	template <typename T>
	concept Is_Trivially_Serializable = (std::is_standard_layout_v<T> && std::is_trivial_v<T>);

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

	template <typename T>
	concept Has_Custom_Serialisation = requires(std::ostream& p_out, std::istream& p_in, T& p_value)
	{
		{ p_value.write_binary(p_out) };
		{ p_value.read_binary(p_in)   };
	};

	template <typename T>
	concept Is_POD_And_Not_Custom_serialisable = !Has_Custom_Serialisation<T> && Is_Trivially_Serializable<T>;


	// Write a custom serialisable object to a binary stream.
	template<Has_Custom_Serialisation T>
	inline void write_binary(std::ostream& p_out, const T& p_value)
	{
		p_value.write_binary(p_out);
	}
	// Read a custom serialisable object from a binary stream.
	template<Has_Custom_Serialisation T>
	inline void read_binary(std::istream& p_in, T& p_value)
	{
		p_value.read_binary(p_in);
	}
	// Write a POD type to a binary stream.
	template<Is_POD_And_Not_Custom_serialisable T>
	inline void write_binary(std::ostream& p_out, const T& p_value)
	{
		p_out.write(reinterpret_cast<const char*>(&p_value), sizeof(p_value));
	}
	// Read a POD type from a binary stream.
	template<Is_POD_And_Not_Custom_serialisable T>
	inline void read_binary(std::istream& p_in, T& p_value)
	{
		p_in.read(reinterpret_cast<char*>(&p_value), sizeof(p_value));
	}
	// Write a container to a binary stream.
	template<Is_Container T>
	inline void write_binary(std::ostream& p_out, const T& p_container)
	{
		// If we are writing a contiguous container, we can write the size and then the data in one go.
		// However in the case the container::value_type is not trivially serialisable, we need to serialise each element individually.

		std::size_t num_of_elements = p_container.size();
		p_out.write(reinterpret_cast<const char*>(&num_of_elements), sizeof(num_of_elements));
		using value_type = typename T::value_type;

		if constexpr (Is_Trivially_Serializable<value_type> && Is_Contiguous_Container<T>)
		{
			p_out.write(reinterpret_cast<const char*>(p_container.data()), num_of_elements * sizeof(value_type));
		}
		else
		{
			for (const auto& element : p_container)
				write_binary(p_out, element);
		}
	}
	// Read a container from a binary stream.
	template<Is_Container T>
	inline void read_binary(std::istream& p_in, T& p_container)
	{
		// Read the size of the container and then read the data in one go.
		// If the container::value_type is not trivially serialisable, we need to deserialise each element individually.

		std::size_t num_of_elements;
		p_in.read(reinterpret_cast<char*>(&num_of_elements), sizeof(num_of_elements));
		p_container.resize(num_of_elements);
		using value_type = typename T::value_type;

		if constexpr (Is_Trivially_Serializable<value_type> && Is_Contiguous_Container<T>)
		{
			p_in.read(reinterpret_cast<char*>(p_container.data()), num_of_elements * sizeof(value_type));
		}
		else
		{
			for (auto& element : p_container)
				read_binary(p_in, element);
		}
	}
} // namespace Utility