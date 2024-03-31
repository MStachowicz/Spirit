#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

#include <fstream>
#include <type_traits>

// Helpers for serialising and deserialising objects.
namespace Utility
{
	// Serializable concept for types that can be written to and read from in binary.
	template <typename T>
	concept Serializable =
		std::is_arithmetic_v<T>
		|| std::is_same_v<T, glm::vec3>
		|| std::is_same_v<T, glm::quat>
		|| std::is_same_v<T, glm::mat4>;

	// Write p_value to p_out in binary.
	template <Serializable T>
	void write_binary(std::ofstream& p_out, const T& p_value)
	{
		p_out.write(reinterpret_cast<const char*>(&p_value), sizeof(T));
	}
	// Read into p_value from p_in in binary.
	template <Serializable T>
	void read_binary(std::ifstream& p_in, T& p_value)
	{
		p_in.read(reinterpret_cast<char*>(&p_value), sizeof(T));
	}
} // namespace Utility