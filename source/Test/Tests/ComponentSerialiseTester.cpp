#include "ComponentSerialiseTester.hpp"

#include "Component/Lights.hpp"
#include "Component/Transform.hpp"
#include "Utility/Logger.hpp"
#include "Utility/Serialise.hpp"

#include <fstream>

namespace Test
{
	// Helper function to serialise and deserialise a component into a file.
	//@param p_to_serialise The component to serialise.
	//@param p_deserialised The component to deserialise into.
	//@return True if the serialisation and deserialisation was successful, false otherwise.
	template<typename ComponentType>
	bool ComponentSerialiseTester::test_serialisation(const ComponentType& p_to_serialise, ComponentType& p_deserialised)
	{
		try
		{
			std::ofstream out("test.bin", std::ios::binary);
			out.exceptions(std::ofstream::failbit | std::ofstream::badbit);
			ComponentType::Serialise(p_to_serialise, out, 0);
			out.close();

			std::ifstream in("test.bin", std::ios::binary);
			in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			p_deserialised = ComponentType::Deserialise(in, 0);
			in.close();
		}
		catch (std::exception& e)
		{
			CHECK_TRUE(false, e.what());
			std::remove("test.bin");
			return false;
		}

		std::remove("test.bin");
		return true;
	}

	// Helper function to test the serialisation and deserialisation of a value.
	//@param p_to_serialise The value to write to a binary file.
	//@param test_name The name of the test.
	//@return True if the serialisation and deserialisation was successful, false otherwise.
	template <typename T>
	bool ComponentSerialiseTester::test_serialisation_utility(const T& p_to_serialise, T& p_deserialised)
	{
		try
		{
			// Write the value to a binary file.
			std::ofstream out("test.bin", std::ios::binary);
			out.exceptions(std::ofstream::failbit | std::ofstream::badbit);
			Utility::write_binary(out, p_to_serialise);
			out.close();

			// Read the value back from the binary file.
			std::ifstream in("test.bin", std::ios::binary);
			in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			Utility::read_binary(in, p_deserialised);
			in.close();
		}
		catch (std::exception& e)
		{
			CHECK_TRUE(false, e.what());
			std::remove("test.bin");
			return false;
		}

		std::remove("test.bin");
		return true;
	}

	void ComponentSerialiseTester::run_unit_tests()
	{
		{SCOPE_SECTION("Utility::Serialise")

			{SCOPE_SECTION("Float");
				float out_float = 3.14f;
				float in_float  = 0.f;
				if (test_serialisation_utility(out_float, in_float))
					CHECK_EQUAL(out_float, in_float, "Float equality");
			}
			{SCOPE_SECTION("String");
				std::string out_string = "Hello, world!";
				std::string in_string  = "";
				if (test_serialisation_utility(out_string, in_string))
					CHECK_EQUAL(out_string, in_string, "String equality");
			}
			{SCOPE_SECTION("Vector<int>"); // vector of integers (POD type).
				std::vector<int> out_vector = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
				std::vector<int> in_vector  = {};
				if (test_serialisation_utility(out_vector, in_vector))
				{
					CHECK_EQUAL(out_vector.size(), in_vector.size(), "Vector<int> size");

					if (out_vector.size() == in_vector.size())
					{
						for (size_t i = 0; i < out_vector.size(); ++i)
							CHECK_EQUAL(out_vector[i], in_vector[i], "Vector<int> element");
					}
				}
			}
			{SCOPE_SECTION("Vector<glm::vec3>"); // vector of glm::vec3 (POD type).
				std::vector<glm::vec3> out_vector = { glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(4.0f, 5.0f, 6.0f), glm::vec3(7.0f, 8.0f, 9.0f) };
				std::vector<glm::vec3> in_vector  = {};
				if (test_serialisation_utility(out_vector, in_vector))
				{
					CHECK_EQUAL(out_vector.size(), in_vector.size(), "Vector<glm::vec3> size");

					if (out_vector.size() == in_vector.size())
					{
						for (size_t i = 0; i < out_vector.size(); ++i)
							CHECK_EQUAL(out_vector[i], in_vector[i], "Vector<glm::vec3> element");
					}
				}
			}
		}

		SCOPE_SECTION("Component serialise");

		{SCOPE_SECTION("Directional light");

			Component::DirectionalLight serialised_light;
			serialised_light.m_direction          = glm::vec3(0.8f, 0.2f, 0.1f);
			serialised_light.m_colour             = glm::vec3(0.7f, 0.4f, 1.0f);
			serialised_light.m_ambient_intensity  = 0.42f;
			serialised_light.m_diffuse_intensity  = 0.7f;
			serialised_light.m_specular_intensity = 0.11f;
			serialised_light.m_shadow_near_plane  = 0.57f;
			serialised_light.m_shadow_far_plane   = 0.2f;
			serialised_light.m_ortho_size         = 0.7f;

			Component::DirectionalLight deserialised_light;
			if (test_serialisation(serialised_light, deserialised_light))
			{
				CHECK_EQUAL(serialised_light.m_direction, deserialised_light.m_direction, "Direction");
				CHECK_EQUAL(serialised_light.m_colour, deserialised_light.m_colour, "Colour");
				CHECK_EQUAL(serialised_light.m_ambient_intensity, deserialised_light.m_ambient_intensity, "Ambient intensity");
				CHECK_EQUAL(serialised_light.m_diffuse_intensity, deserialised_light.m_diffuse_intensity, "Diffuse intensity");
				CHECK_EQUAL(serialised_light.m_specular_intensity, deserialised_light.m_specular_intensity, "Specular intensity");
				CHECK_EQUAL(serialised_light.m_shadow_near_plane, deserialised_light.m_shadow_near_plane, "Shadow near plane");
				CHECK_EQUAL(serialised_light.m_shadow_far_plane, deserialised_light.m_shadow_far_plane, "Shadow far plane");
				CHECK_EQUAL(serialised_light.m_ortho_size, deserialised_light.m_ortho_size, "Ortho size");
			}
		}

		{SCOPE_SECTION("Point light");

			Component::PointLight serialised_light;
			serialised_light.m_position           = glm::vec3(0.8f, 0.2f, 0.1f);
			serialised_light.m_colour             = glm::vec3(0.7f, 0.4f, 1.0f);
			serialised_light.m_ambient_intensity  = 0.42f;
			serialised_light.m_diffuse_intensity  = 0.7f;
			serialised_light.m_specular_intensity = 0.11f;
			serialised_light.m_constant           = 0.57f;
			serialised_light.m_linear             = 0.2f;
			serialised_light.m_quadratic          = 0.7f;

			Component::PointLight deserialised_light;
			if (test_serialisation(serialised_light, deserialised_light))
			{
				CHECK_EQUAL(serialised_light.m_position, deserialised_light.m_position, "Position");
				CHECK_EQUAL(serialised_light.m_colour, deserialised_light.m_colour, "Colour");
				CHECK_EQUAL(serialised_light.m_ambient_intensity, deserialised_light.m_ambient_intensity, "Ambient intensity");
				CHECK_EQUAL(serialised_light.m_diffuse_intensity, deserialised_light.m_diffuse_intensity, "Diffuse intensity");
				CHECK_EQUAL(serialised_light.m_specular_intensity, deserialised_light.m_specular_intensity, "Specular intensity");
				CHECK_EQUAL(serialised_light.m_constant, deserialised_light.m_constant, "Constant");
				CHECK_EQUAL(serialised_light.m_linear, deserialised_light.m_linear, "Linear");
				CHECK_EQUAL(serialised_light.m_quadratic, deserialised_light.m_quadratic, "Quadratic");
			}
		}

		{SCOPE_SECTION("Spotlight");

			Component::SpotLight serialised_light;
			serialised_light.m_position           = glm::vec3(0.8f, 0.2f, 0.1f);
			serialised_light.m_direction          = glm::vec3(0.7f, 0.4f, 1.0f);
			serialised_light.m_colour             = glm::vec3(0.7f, 0.4f, 1.0f);
			serialised_light.m_ambient_intensity  = 0.42f;
			serialised_light.m_diffuse_intensity  = 0.7f;
			serialised_light.m_specular_intensity = 0.11f;
			serialised_light.m_constant           = 0.57f;
			serialised_light.m_linear             = 0.2f;
			serialised_light.m_quadratic          = 0.7f;
			serialised_light.m_cutoff             = 0.5f;
			serialised_light.m_outer_cutoff       = 0.7f;

			Component::SpotLight deserialised_light;
			if (test_serialisation(serialised_light, deserialised_light))
			{
				CHECK_EQUAL(serialised_light.m_position, deserialised_light.m_position, "Position");
				CHECK_EQUAL(serialised_light.m_direction, deserialised_light.m_direction, "Direction");
				CHECK_EQUAL(serialised_light.m_colour, deserialised_light.m_colour, "Colour");
				CHECK_EQUAL(serialised_light.m_ambient_intensity, deserialised_light.m_ambient_intensity, "Ambient intensity");
				CHECK_EQUAL(serialised_light.m_diffuse_intensity, deserialised_light.m_diffuse_intensity, "Diffuse intensity");
				CHECK_EQUAL(serialised_light.m_specular_intensity, deserialised_light.m_specular_intensity, "Specular intensity");
				CHECK_EQUAL(serialised_light.m_constant, deserialised_light.m_constant, "Constant");
				CHECK_EQUAL(serialised_light.m_linear, deserialised_light.m_linear, "Linear");
				CHECK_EQUAL(serialised_light.m_quadratic, deserialised_light.m_quadratic, "Quadratic");
				CHECK_EQUAL(serialised_light.m_cutoff, deserialised_light.m_cutoff, "Cutoff");
				CHECK_EQUAL(serialised_light.m_outer_cutoff, deserialised_light.m_outer_cutoff, "Outer cutoff");
			}
		}

		{SCOPE_SECTION("Transform");
			Component::Transform serialised_transform;
			serialised_transform.m_position       = glm::vec3(0.5f, 0.1f, 0.4f);
			serialised_transform.m_scale          = glm::vec3(2.4f, 2.3f, 5.0f);
			serialised_transform.m_orientation    = glm::quat(3.8f, 2.5f, 4.4f, 0.5f);

			Component::Transform deserialised_transform;
			if (test_serialisation(serialised_transform, deserialised_transform))
			{
				CHECK_EQUAL(serialised_transform.m_position, deserialised_transform.m_position, "Position");
				CHECK_EQUAL(serialised_transform.m_scale, deserialised_transform.m_scale, "Scale");
				CHECK_EQUAL(serialised_transform.m_orientation, deserialised_transform.m_orientation, "Orientation");
			}
		}
	}

	void ComponentSerialiseTester::run_performance_tests()
	{
	}
} // namespace Test