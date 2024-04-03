#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

#include <fstream>
#include <type_traits>

// Helpers for serialising and deserialising objects.
namespace Utility
{
	// Serializable concept for keeping track of types that can be serialised into binary.
	template <typename T>
	concept Serializable_Type =
		std::is_arithmetic_v<T>
		|| std::is_same_v<T, glm::vec3>
		|| std::is_same_v<T, glm::quat>
		|| std::is_same_v<T, glm::mat4>;

	// Write p_value to p_out in binary.
	template <typename T>
	void write_binary(std::ofstream& p_out, const T& p_value)
	{
		static_assert(sizeof(T) > 0, "Cannot write a type of size 0");
		static_assert(!std::is_pointer_v<T> && !std::is_reference_v<T>, "Cannot write a reference or pointer");
		static_assert(Serializable_Type<T>, "Type is not in serializable list");

		p_out.write(reinterpret_cast<const char*>(&p_value), sizeof(T));
	}
	// Read into p_value from p_in in binary.
	template <typename T>
	void read_binary(std::ifstream& p_in, T& p_value)
	{
		static_assert(sizeof(T) > 0, "Cannot read into a type of size 0");
		static_assert(!std::is_pointer_v<T> && !std::is_reference_v<T>, "Cannot read into a reference or pointer");
		static_assert(Serializable_Type<T>, "Type is not in serializable list");

		p_in.read(reinterpret_cast<char*>(&p_value), sizeof(T));
	}
	template <>
	inline void write_binary<std::string>(std::ofstream& p_out, const std::string& p_value)
	{
		// Write the size of the string first then write the string data
		std::size_t size = p_value.size();
		p_out.write(reinterpret_cast<const char*>(&size), sizeof(size));
		p_out.write(p_value.data(), p_value.size());
	}
	template <>
	inline void read_binary<std::string>(std::ifstream& p_in, std::string& p_value)
	{
		// Read the size of the string first then read the string data
		std::size_t size;
		p_in.read(reinterpret_cast<char*>(&size), sizeof(size));
		p_value.resize(size);
		p_in.read(p_value.data(), size);
	}
} // namespace Utility