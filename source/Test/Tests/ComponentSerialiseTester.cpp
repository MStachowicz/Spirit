#include "ComponentSerialiseTester.hpp"

#include "Component/Lights.hpp"
#include "Utility/Logger.hpp"

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
		catch (std::ofstream::failure& e)
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
	}

	void ComponentSerialiseTester::run_performance_tests()
	{
	}
} // namespace Test