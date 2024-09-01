#include "ComponentSerialiseTester.hpp"

#include "Component/Lights.hpp"
#include "Component/Transform.hpp"
#include "Utility/Logger.hpp"
#include "Utility/Serialise.hpp"

#include <iostream>
#include <list>
#include <string>
#include <vector>

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
			ComponentType::serialise(out, 0, p_to_serialise);
			out.close();

			std::ifstream in("test.bin", std::ios::binary);
			in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			p_deserialised = ComponentType::deserialise(in, 0);
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
			Utility::write_binary(out, 0, p_to_serialise);
			out.close();

			// Read the value back from the binary file.
			std::ifstream in("test.bin", std::ios::binary);
			in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			Utility::read_binary(in, 0, p_deserialised);
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
	{SCOPE_SECTION("Serialise/Deserialise")

		// Test the serialisation of a POD types.
		{SCOPE_SECTION("Plain old data")
			{SCOPE_SECTION("Float");
				float out_float = 3.14f;
				float in_float  = 0.f;
				if (test_serialisation_utility(out_float, in_float))
					CHECK_EQUAL(out_float, in_float, "Equality");
			}
			{SCOPE_SECTION("int");
				int out_int = 42;
				int in_int  = 0;
				if (test_serialisation_utility(out_int, in_int))
					CHECK_EQUAL(out_int, in_int, "Equality");
			}
			{SCOPE_SECTION("unsigned int");
				unsigned int out_uint = 42;
				unsigned int in_uint  = 0;
				if (test_serialisation_utility(out_uint, in_uint))
					CHECK_EQUAL(out_uint, in_uint, "Equality");
			}
			{SCOPE_SECTION("char");
				char out_char = 'a';
				char in_char  = '\0';
				if (test_serialisation_utility(out_char, in_char))
					CHECK_EQUAL(out_char, in_char, "Equality");
			}
			{SCOPE_SECTION("unsigned char");
				unsigned char out_uchar = 'a';
				unsigned char in_uchar  = '\0';
				if (test_serialisation_utility(out_uchar, in_uchar))
					CHECK_EQUAL(out_uchar, in_uchar, "Equality");
			}
			{SCOPE_SECTION("bool");
				bool out_bool = true;
				bool in_bool  = false;
				if (test_serialisation_utility(out_bool, in_bool))
					CHECK_EQUAL(out_bool, in_bool, "Equality");
			}
			{SCOPE_SECTION("short");
				short out_short = 42;
				short in_short  = 0;
				if (test_serialisation_utility(out_short, in_short))
					CHECK_EQUAL(out_short, in_short, "Equality");
			}
			{SCOPE_SECTION("long");
				long out_long = 42;
				long in_long  = 0;
				if (test_serialisation_utility(out_long, in_long))
					CHECK_EQUAL(out_long, in_long, "Equality");
			}
			{SCOPE_SECTION("long long");
				long long out_long_long = 42;
				long long in_long_long  = 0;
				if (test_serialisation_utility(out_long_long, in_long_long))
					CHECK_EQUAL(out_long_long, in_long_long, "Equality");
			}
		}

		{SCOPE_SECTION("Non-POD")
			{SCOPE_SECTION("string")
				std::string out_string = "Hello, world!";
				std::string in_string  = "";
				if (test_serialisation_utility(out_string, in_string))
					CHECK_EQUAL(out_string, in_string, "Equality");
			}
		}

		{SCOPE_SECTION("Container")
			{SCOPE_SECTION("POD")
				{SCOPE_SECTION("Contiguous")
					{SCOPE_SECTION("Vector<int>")
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
					{SCOPE_SECTION("Vector<float>")
						std::vector<float> out_vector = { 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f, 10.10f };
						std::vector<float> in_vector  = {};
						if (test_serialisation_utility(out_vector, in_vector))
						{
							CHECK_EQUAL(out_vector.size(), in_vector.size(), "Vector<float> size");
							if (out_vector.size() == in_vector.size())
							{
								for (size_t i = 0; i < out_vector.size(); ++i)
									CHECK_EQUAL(out_vector[i], in_vector[i], "Vector<float> element");
							}
						}
					}
					{SCOPE_SECTION("Array<int>")
						// std::array<int, 10> out_array = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
						// std::array<int, 10> in_array  = {};
						// if (test_serialisation_utility(out_array, in_array))
						// {
						// 	for (size_t i = 0; i < out_array.size(); ++i)
						// 		CHECK_EQUAL(out_array[i], in_array[i], "Array<int> element");
						// }
					}
				}
				{SCOPE_SECTION("Non-Contiguous")
					{SCOPE_SECTION("List<int>")
						std::list<int> out_list = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
						std::list<int> in_list  = {};
						if (test_serialisation_utility(out_list, in_list))
						{
							CHECK_EQUAL(out_list.size(), in_list.size(), "List<int> size");

							auto out_it = out_list.begin();
							auto in_it  = in_list.begin();
							while (out_it != out_list.end() && in_it != in_list.end())
							{
								CHECK_EQUAL(*out_it, *in_it, "List<int> element");
								++out_it;
								++in_it;
							}
						}
					}
					{SCOPE_SECTION("List<float>")
						std::list<float> out_list = { 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f, 10.10f };
						std::list<float> in_list  = {};
						if (test_serialisation_utility(out_list, in_list))
						{
							CHECK_EQUAL(out_list.size(), in_list.size(), "List<float> size");

							auto out_it = out_list.begin();
							auto in_it  = in_list.begin();
							while (out_it != out_list.end() && in_it != in_list.end())
							{
								CHECK_EQUAL(*out_it, *in_it, "List<float> element");
								++out_it;
								++in_it;
							}
						}
					}
				}
			}
			{SCOPE_SECTION("Non-POD")
				{SCOPE_SECTION("Contiguous")
					{SCOPE_SECTION("Vector<string>")
						std::vector<std::string> out_vector = { "Hello", "World", "!" };
						std::vector<std::string> in_vector  = {};
						if (test_serialisation_utility(out_vector, in_vector))
						{
							CHECK_EQUAL(out_vector.size(), in_vector.size(), "Vector<string> size");
							if (out_vector.size() == in_vector.size())
							{
								for (size_t i = 0; i < out_vector.size(); ++i)
									CHECK_EQUAL(out_vector[i], in_vector[i], "Vector<string> element");
							}
						}
					}
				}
				{SCOPE_SECTION("Non-contiguous")
					{SCOPE_SECTION("List<string>")
						std::list<std::string> out_list = { "Hello", "World", "!" };
						std::list<std::string> in_list  = {};
						if (test_serialisation_utility(out_list, in_list))
						{
							CHECK_EQUAL(out_list.size(), in_list.size(), "List<string> size");

							auto out_it = out_list.begin();
							auto in_it  = in_list.begin();
							while (out_it != out_list.end() && in_it != in_list.end())
							{
								CHECK_EQUAL(*out_it, *in_it, "List<string> element");
								++out_it;
								++in_it;
							}
						}
					}
				}
			}
			{SCOPE_SECTION("Nested")
				{SCOPE_SECTION("POD") // Leaf type is POD.
					{SCOPE_SECTION("Double contiguous")
						{SCOPE_SECTION("Vector<Vector<int>>")
							std::vector<std::vector<int>> out_vector = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
							std::vector<std::vector<int>> in_vector  = {};
							if (test_serialisation_utility(out_vector, in_vector))
							{
								CHECK_EQUAL(out_vector.size(), in_vector.size(), "Vector<Vector<int>> size");
								if (out_vector.size() == in_vector.size())
								{
									for (size_t i = 0; i < out_vector.size(); ++i)
									{
										CHECK_EQUAL(out_vector[i].size(), in_vector[i].size(), "Vector<int> size");
										if (out_vector[i].size() == in_vector[i].size())
										{
											for (size_t j = 0; j < out_vector[i].size(); ++j)
												CHECK_EQUAL(out_vector[i][j], in_vector[i][j], "Vector<int> element");
										}
									}
								}
							}
						}
					}
					{SCOPE_SECTION("Non-contiguous inside contiguous")
						{SCOPE_SECTION("Vector<List<int>>")
							std::vector<std::list<int>> out_vector = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
							std::vector<std::list<int>> in_vector  = {};
							if (test_serialisation_utility(out_vector, in_vector))
							{
								CHECK_EQUAL(out_vector.size(), in_vector.size(), "Vector<List<int>> size");
								if (out_vector.size() == in_vector.size())
								{
									for (size_t i = 0; i < out_vector.size(); ++i)
									{
										CHECK_EQUAL(out_vector[i].size(), in_vector[i].size(), "List<int> size");
										if (out_vector[i].size() == in_vector[i].size())
										{
											auto out_it = out_vector[i].begin();
											auto in_it  = in_vector[i].begin();
											while (out_it != out_vector[i].end() && in_it != in_vector[i].end())
											{
												CHECK_EQUAL(*out_it, *in_it, "List<int> element");
												++out_it;
												++in_it;
											}
										}
									}
								}
							}
						}
					}
					{SCOPE_SECTION("Contiguous inside non-contiguous")
						{SCOPE_SECTION("List<vector<int>>")
							std::list<std::vector<int>> out_list = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
							std::list<std::vector<int>> in_list  = {};
							if (test_serialisation_utility(out_list, in_list))
							{
								CHECK_EQUAL(out_list.size(), in_list.size(), "List<Vector<int>> size");

								auto out_it = out_list.begin();
								auto in_it  = in_list.begin();
								while (out_it != out_list.end() && in_it != in_list.end())
								{
									CHECK_EQUAL(out_it->size(), in_it->size(), "Vector<int> size");
									if (out_it->size() == in_it->size())
									{
										auto out_vec_it = out_it->begin();
										auto in_vec_it  = in_it->begin();
										while (out_vec_it != out_it->end() && in_vec_it != in_it->end())
										{
											CHECK_EQUAL(*out_vec_it, *in_vec_it, "Vector<int> element");
											++out_vec_it;
											++in_vec_it;
										}
									}
									++out_it;
									++in_it;
								}
							}
						}
					}
					{SCOPE_SECTION("Double non-contiguous")
						std::list<std::list<int>> out_list = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
						std::list<std::list<int>> in_list  = {};
						if (test_serialisation_utility(out_list, in_list))
						{
							CHECK_EQUAL(out_list.size(), in_list.size(), "List<List<int>> size");

							auto out_it = out_list.begin();
							auto in_it  = in_list.begin();
							while (out_it != out_list.end() && in_it != in_list.end())
							{
								CHECK_EQUAL(out_it->size(), in_it->size(), "List<int> size");
								if (out_it->size() == in_it->size())
								{
									auto out_vec_it = out_it->begin();
									auto in_vec_it  = in_it->begin();
									while (out_vec_it != out_it->end() && in_vec_it != in_it->end())
									{
										CHECK_EQUAL(*out_vec_it, *in_vec_it, "List<int> element");
										++out_vec_it;
										++in_vec_it;
									}
								}
								++out_it;
								++in_it;
							}
						}
					}
				}
				{SCOPE_SECTION("Non-POD") // Leaf type is non-POD (string).
					{SCOPE_SECTION("Double contiguous")
						{SCOPE_SECTION("Vector<Vector<string>>")
							std::vector<std::vector<std::string>> out_vector = { { "Hello", "World", "!" }, { "Goodbye", "World", "!" }, { "Hello", "World", "Goodbye" } };
							std::vector<std::vector<std::string>> in_vector  = {};

							if (test_serialisation_utility(out_vector, in_vector))
							{
								CHECK_EQUAL(out_vector.size(), in_vector.size(), "Vector<Vector<string>> size");
								if (out_vector.size() == in_vector.size())
								{
									for (size_t i = 0; i < out_vector.size(); ++i)
									{
										CHECK_EQUAL(out_vector[i].size(), in_vector[i].size(), "Vector<string> size");
										if (out_vector[i].size() == in_vector[i].size())
										{
											for (size_t j = 0; j < out_vector[i].size(); ++j)
												CHECK_EQUAL(out_vector[i][j], in_vector[i][j], "Vector<string> element");
										}
									}
								}
							}
						}
					}
					{SCOPE_SECTION("Non-contiguous inside contiguous")
						{SCOPE_SECTION("Vector<List<string>>")
							std::vector<std::list<std::string>> out_vector = { { "Hello", "World", "!" }, { "Goodbye", "World", "!" }, { "Hello", "World", "Goodbye" } };
							std::vector<std::list<std::string>> in_vector  = {};

							if (test_serialisation_utility(out_vector, in_vector))
							{
								CHECK_EQUAL(out_vector.size(), in_vector.size(), "Vector<List<string>> size");
								if (out_vector.size() == in_vector.size())
								{
									for (size_t i = 0; i < out_vector.size(); ++i)
									{
										CHECK_EQUAL(out_vector[i].size(), in_vector[i].size(), "List<string> size");
										if (out_vector[i].size() == in_vector[i].size())
										{
											auto out_it = out_vector[i].begin();
											auto in_it  = in_vector[i].begin();
											while (out_it != out_vector[i].end() && in_it != in_vector[i].end())
											{
												CHECK_EQUAL(*out_it, *in_it, "List<string> element");
												++out_it;
												++in_it;
											}
										}
									}
								}
							}
						}
					}
					{SCOPE_SECTION("Contiguous inside non-contiguous")
						{SCOPE_SECTION("List<vector<string>>")
							std::list<std::vector<std::string>> out_list = { { "Hello", "World", "!" }, { "Goodbye", "World", "!" }, { "Hello", "World", "Goodbye" } };
							std::list<std::vector<std::string>> in_list  = {};

							if (test_serialisation_utility(out_list, in_list))
							{
								CHECK_EQUAL(out_list.size(), in_list.size(), "List<Vector<string>> size");

								auto out_it = out_list.begin();
								auto in_it  = in_list.begin();
								while (out_it != out_list.end() && in_it != in_list.end())
								{
									CHECK_EQUAL(out_it->size(), in_it->size(), "Vector<string> size");
									if (out_it->size() == in_it->size())
									{
										auto out_vec_it = out_it->begin();
										auto in_vec_it  = in_it->begin();
										while (out_vec_it != out_it->end() && in_vec_it != in_it->end())
										{
											CHECK_EQUAL(*out_vec_it, *in_vec_it, "Vector<string> element");
											++out_vec_it;
											++in_vec_it;
										}
									}
									++out_it;
									++in_it;
								}
							}
						}
					}
					{SCOPE_SECTION("Double non-contiguous")
						std::list<std::list<std::string>> out_list = { { "Hello", "World", "!" }, { "Goodbye", "World", "!" }, { "Hello", "World", "Goodbye" } };
						std::list<std::list<std::string>> in_list  = {};

						if (test_serialisation_utility(out_list, in_list))
						{
							CHECK_EQUAL(out_list.size(), in_list.size(), "List<List<string>> size");

							auto out_it = out_list.begin();
							auto in_it  = in_list.begin();
							while (out_it != out_list.end() && in_it != in_list.end())
							{
								CHECK_EQUAL(out_it->size(), in_it->size(), "List<string> size");
								if (out_it->size() == in_it->size())
								{
									auto out_vec_it = out_it->begin();
									auto in_vec_it  = in_it->begin();
									while (out_vec_it != out_it->end() && in_vec_it != in_it->end())
									{
										CHECK_EQUAL(*out_vec_it, *in_vec_it, "List<string> element");
										++out_vec_it;
										++in_vec_it;
									}
								}
								++out_it;
								++in_it;
							}
						}
					}
				}
			}
		}

		// Test user defined serialisation.
		{SCOPE_SECTION("Custom serialisation")
			{SCOPE_SECTION("POD");
				// Plain old data with custom serialisation.
				struct Custom_Serialisation_POD
				{
					int i;
					void write_binary(std::ostream& p_out, uint16_t p_version) const { Utility::write_binary(p_out, p_version, i); }
					void read_binary(std::istream& p_in, uint16_t p_version)         { Utility::read_binary(p_in, p_version, i);   }
				};
				static_assert(Utility::Is_Trivially_Serializable<Custom_Serialisation_POD>, "Custom_Serialisation_POD must be trivially serializable");
				static_assert(Utility::Has_Custom_Serialisation<Custom_Serialisation_POD>, "Custom_Serialisation_POD must have custom serialisation");

				{SCOPE_SECTION("Single object");
					Custom_Serialisation_POD out_pod{ 42 };
					Custom_Serialisation_POD in_pod;
					if (test_serialisation_utility(out_pod, in_pod))
						CHECK_EQUAL(out_pod.i, in_pod.i, "Equality");
				}
				{SCOPE_SECTION("Container")
					{SCOPE_SECTION("Contiguous")
						{SCOPE_SECTION("Vector")
							std::vector<Custom_Serialisation_POD> out_vector = { Custom_Serialisation_POD{ 1 }, Custom_Serialisation_POD{ 2 }, Custom_Serialisation_POD{ 3 } };
							std::vector<Custom_Serialisation_POD> in_vector  = {};
							if (test_serialisation_utility(out_vector, in_vector))
							{
								CHECK_EQUAL(out_vector.size(), in_vector.size(), "Vector size");

								if (out_vector.size() == in_vector.size())
								{
									for (size_t i = 0; i < out_vector.size(); ++i)
										CHECK_EQUAL(out_vector[i].i, in_vector[i].i, "Vector element");
								}
							}
						}
					}
					{SCOPE_SECTION("Non-contiguous")
						{SCOPE_SECTION("List")
							std::list<Custom_Serialisation_POD> out_list = { Custom_Serialisation_POD{ 1 }, Custom_Serialisation_POD{ 2 }, Custom_Serialisation_POD{ 3 } };
							std::list<Custom_Serialisation_POD> in_list  = {};
							if (test_serialisation_utility(out_list, in_list))
							{
								CHECK_EQUAL(out_list.size(), in_list.size(), "List size");

								auto out_it = out_list.begin();
								auto in_it  = in_list.begin();
								while (out_it != out_list.end() && in_it != in_list.end())
								{
									CHECK_EQUAL(out_it->i, in_it->i, "List element");
									++out_it;
									++in_it;
								}
							}
						}
					}
				}
			}
			{SCOPE_SECTION("Non-POD")
				// Non-POD type with custom serialisation.
				struct Custom_serialisation_Non_POD
				{
					std::string c;
					void write_binary(std::ostream& p_out, uint16_t p_version) const { Utility::write_binary(p_out, p_version, c); }
					void read_binary(std::istream& p_in, uint16_t p_version)         { Utility::read_binary(p_in, p_version, c);   }
				};
				static_assert(!Utility::Is_Trivially_Serializable<Custom_serialisation_Non_POD>, "Custom_serialisation_Non_POD must not be trivially serializable");
				static_assert(Utility::Has_Custom_Serialisation<Custom_serialisation_Non_POD>, "Custom_serialisation_Non_POD must have custom serialisation");

				{SCOPE_SECTION("Single object");
					Custom_serialisation_Non_POD out_non_pod;
					out_non_pod.c = "Hello, world!";
					Custom_serialisation_Non_POD in_non_pod;
					if (test_serialisation_utility(out_non_pod, in_non_pod))
						CHECK_EQUAL(out_non_pod.c, in_non_pod.c, "Equality");
				}
				{SCOPE_SECTION("Container")
					{SCOPE_SECTION("Contiguous")
						std::vector<Custom_serialisation_Non_POD> out_vector = { Custom_serialisation_Non_POD{ "Hello" }, Custom_serialisation_Non_POD{ "World" }, Custom_serialisation_Non_POD{ "!" } };
						std::vector<Custom_serialisation_Non_POD> in_vector  = {};
						if (test_serialisation_utility(out_vector, in_vector))
						{
							CHECK_EQUAL(out_vector.size(), in_vector.size(), "Vector size");

							if (out_vector.size() == in_vector.size())
							{
								for (size_t i = 0; i < out_vector.size(); ++i)
									CHECK_EQUAL(out_vector[i].c, in_vector[i].c, "Vector element");
							}
						}
					}
					{SCOPE_SECTION("Non-contiguous")
						std::list<Custom_serialisation_Non_POD> out_list = { Custom_serialisation_Non_POD{ "Hello" }, Custom_serialisation_Non_POD{ "World" }, Custom_serialisation_Non_POD{ "!" } };
						std::list<Custom_serialisation_Non_POD> in_list  = {};
						if (test_serialisation_utility(out_list, in_list))
						{
							CHECK_EQUAL(out_list.size(), in_list.size(), "List size");

							auto out_it = out_list.begin();
							auto in_it  = in_list.begin();
							while (out_it != out_list.end() && in_it != in_list.end())
							{
								CHECK_EQUAL(out_it->c, in_it->c, "List element");
								++out_it;
								++in_it;
							}
						}
					}
				}
			}
		}
		{SCOPE_SECTION("ECS components");
			{SCOPE_SECTION("Directional light");
				static_assert(Utility::Is_Serializable_v<Component::DirectionalLight>, "DirectionalLight must have custom serialisation");

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
				static_assert(Utility::Is_Serializable_v<Component::PointLight>, "PointLight must have custom serialisation");

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
				static_assert(Utility::Is_Serializable_v<Component::SpotLight>, "SpotLight must have custom serialisation");

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
				static_assert(Utility::Is_Serializable_v<Component::Transform>, "Transform must have custom serialisation");

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
	}

	void ComponentSerialiseTester::run_performance_tests()
	{
	}
} // namespace Test