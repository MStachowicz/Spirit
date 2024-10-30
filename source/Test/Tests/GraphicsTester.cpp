#include "GraphicsTester.hpp"

#include "OpenGL/Types.hpp"
#include "OpenGL/Shader.hpp"
#include "OpenGL/GLState.hpp"
#include "OpenGL/DrawCall.hpp"

#include "Platform/Core.hpp"
#include "Platform/Input.hpp"
#include "Platform/Window.hpp"

namespace Test
{
	void GraphicsTester::run_unit_tests()
	{
		Platform::Core::initialise_directories();
		Platform::Core::initialise_GLFW();
		Platform::Input input   = Platform::Input();
		Platform::Window window = Platform::Window({0.5, 0.5}, input);
		Platform::Core::initialise_OpenGL();


		{SCOPE_SECTION("Compute")
			{SCOPE_SECTION("Increment")
				OpenGL::Buffer in_buffer = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit});
				std::array<unsigned int, 8> data = { 1, 2, 3, 4, 5, 6, 7, 8 };
				in_buffer.upload_data(data);

				OpenGL::Buffer out_buffer = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit});
				out_buffer.upload_data(std::array<unsigned int, 8>{0, 0, 0, 0, 0, 0, 0, 0});

				OpenGL::Shader shader = OpenGL::Shader("increment.comp");
				OpenGL::DrawCall compute_call;
				compute_call.set_SSBO("DataIn", in_buffer);
				compute_call.set_SSBO("DataOut", out_buffer);
				compute_call.submit_compute(shader, 8, 1, 1); // data.size()

				OpenGL::memory_barrier({OpenGL::MemoryBarrierFlag::ShaderStorageBarrierBit});

				std::array<unsigned int, 8> expected = { 2, 3, 4, 5, 6, 7, 8, 9 };
				auto result = out_buffer.download_data<unsigned int>(expected.size());

				for (size_t i = 0; i < expected.size(); ++i)
					CHECK_EQUAL(result[i], expected[i], "Increment");
			}
			{SCOPE_SECTION("global_sum")
				OpenGL::Buffer in_buffer = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit});
				std::array<unsigned int, 8> data = { 3, 1, 7, 0, 4, 1, 6, 3 };
				                            //0 // 4  7  5  9
				                            //1 // 11 14
				                            //2 // 25
				in_buffer.upload_data(data);
				if (data.size() % 2 != 0) throw std::runtime_error("Data size must be a power of 2");

				OpenGL::Buffer out_buffer = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit});
				out_buffer.upload_data(std::array<unsigned int, 8>{0, 0, 0, 0, 0, 0, 0, 0});

				OpenGL::Shader shader                            = OpenGL::Shader("global_sum.comp");
				std::array<std::vector<int>, 3> expected_results = {{ {{ 4, 7, 5, 9 }}, {{ 11, 14 }}, {{ 25 }} }};

				size_t reduction_steps = std::log2(data.size());
				for (size_t i = 0; i < reduction_steps; ++i)
				{
					OpenGL::DrawCall compute_call;
					compute_call.set_SSBO("DataIn",  i % 2 == 0 ? in_buffer : out_buffer);
					compute_call.set_SSBO("DataOut", i % 2 == 0 ? out_buffer : in_buffer);
					compute_call.submit_compute(shader, data.size() / (1 << (i + 1)), 1, 1); // 4, 2, 1 with 8 elements
					OpenGL::memory_barrier({OpenGL::MemoryBarrierFlag::ShaderStorageBarrierBit});

					auto result = (i % 2 == 0 ? out_buffer : in_buffer).download_data<unsigned int>(data.size());
					for (size_t j = 0; j < expected_results[i].size(); ++j)
						CHECK_EQUAL(result[j], expected_results[i][j], "Reduction step " + std::to_string(i));
				}
			}
			{SCOPE_SECTION("Prefix Sum")
				// Prefix sum is calculated in two passes.
				// In the first pass we calculate the binary tree of global sum elements with our input data forming the leaf nodes.
				// In the second pass we take the binary tree global sum data and work root -> leaf and calculate the prefix sum at each node.

				OpenGL::Buffer buff = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit});
				std::array<unsigned int, 16> data = { 0, 0, 0, 0, 0, 0, 0,    3, 1, 7, 0, 4, 1, 6, 3, 0 }; // last 0 is padding
				if (data.size() % 2 != 0) throw std::runtime_error("Data size must be a power of 2");
				buff.upload_data(data);

				{SCOPE_SECTION("First pass - Global sum") // Global sum pass
					OpenGL::Shader shader = OpenGL::Shader("prefix_sum_first_pass.comp");

					std::vector<std::vector<unsigned int>> expected_results = {
						 { 0,  0,  0,  4, 7, 5, 9,    3, 1, 7, 0, 4, 1, 6, 3, 0 },  // Reduction 1
						 { 0,  11, 14, 4, 7, 5, 9,    3, 1, 7, 0, 4, 1, 6, 3, 0 },  // Reduction 2
						 { 25, 11, 14, 4, 7, 5, 9,    3, 1, 7, 0, 4, 1, 6, 3, 0 }}; // Reduction 3

					size_t reduction_steps = std::log2(data.size()) - 1;
					for (size_t i = 0; i < reduction_steps; ++i)
					{
						unsigned int node_count = data.size() / (1 << (i + 2)); // Nodes to be calculated this reduction step:     4, 2, 1 with 8 elements
						unsigned int offset     = node_count - 1;               // Offset into the data for writing the reduction: 3, 2, 1 with 8 elements

						OpenGL::DrawCall compute_call;
						compute_call.set_SSBO("DataIn", buff);
						compute_call.set_uniform("offset", offset);
						compute_call.submit_compute(shader, node_count, 1, 1);
						OpenGL::memory_barrier({OpenGL::MemoryBarrierFlag::ShaderStorageBarrierBit});

						auto result = buff.download_data<unsigned int>(data.size());
						for (size_t j = 0; j < expected_results[i].size(); ++j)
							CHECK_EQUAL(result[j], expected_results[i][j], "Reduction step " + std::to_string(i));
					}
				}

				{SCOPE_SECTION("Second pass - Prefix sum")
					OpenGL::Shader shader = OpenGL::Shader("prefix_sum_second_pass.comp");
					OpenGL::Buffer prefix_sum_buffer = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit});
					prefix_sum_buffer.upload_data(std::vector<unsigned int>(data.size(), 0)); // Must be initialised to 0 for root node to be correct.

					std::vector<std::vector<unsigned int>> expected_results = {
						{ 0, 0, 11, 0, 0,  0,  0,    0, 0, 0,  0,  0,  0,  0,  0, 0 },
						{ 0, 0, 11, 0, 4, 11, 16,    0, 0, 0,  0,  0,  0,  0,  0, 0 },
						{ 0, 0, 11, 0, 4, 11, 16,    0, 3, 4,  11, 11, 15, 16, 22, 0 }};

					size_t scan_steps = std::log2(data.size()) - 1;
					for (size_t i = 0; i < scan_steps; ++i)
					{
						unsigned int node_count = 1 << i;         // Nodes to be calculated this reduction step:     1, 2, 4 with 8 elements
						unsigned int offset     = node_count - 1; // Offset into the data for writing the reduction: 0, 1, 3 with 8 elements

						OpenGL::DrawCall compute_call;
						compute_call.set_SSBO("GlobalSum", buff);
						compute_call.set_SSBO("PrefixSum", prefix_sum_buffer);
						compute_call.set_uniform("offset", offset);
						compute_call.submit_compute(shader, node_count, 1, 1);
						OpenGL::memory_barrier({OpenGL::MemoryBarrierFlag::ShaderStorageBarrierBit});

						auto result = prefix_sum_buffer.download_data<unsigned int>(data.size());
						for (size_t j = 0; j < expected_results[i].size(); ++j)
							CHECK_EQUAL(result[j], expected_results[i][j], "Expansion step " + std::to_string(i));
					}

					{
						//std::array<unsigned int, 8> inclusive_out; // Result: { 3, 4, 11, 11, 15, 16, 22, 25}
						//std::array<unsigned int, 8> exclusive_out; // Result: { 0, 3, 4,  11, 11, 15, 16, 22}
						//std::array<unsigned int, 8> data_in         = { 3, 1, 7, 0, 4, 1, 6, 3 };
						//std::inclusive_scan(data_in.begin(), data_in.end(), inclusive_out.begin(), std::plus<>());
						//std::exclusive_scan(data_in.begin(), data_in.end(), exclusive_out.begin(), 0, std::plus<>());

						// If we want to inclusive prefix sum we need to run this final step of copying the

						// First N - 1 elements are the non-leaf nodes of the prefix sum tree
						// Elements N -> N + N are the exclsuive sum
						// Elements N + 1 -> N + N + 1 are the inclusive sum
						std::array<unsigned int, 16> expected_final = { 0, 0, 11, 0, 4, 11, 16,    0, 3, 4, 11, 11, 15, 16, 22,   25 };

						// Global sum buffer (buff) containts the final prefix sum as its 0th element.
						// Copy this into the end index of the prefix sum result.
						prefix_sum_buffer.copy_sub_data(buff, 0, sizeof(unsigned int) * (data.size() - 1), sizeof(unsigned int));
						auto result = prefix_sum_buffer.download_data<unsigned int>(data.size());

						// Check the result
						for (size_t i = 0; i < expected_final.size(); i++)
							CHECK_EQUAL(result[i], expected_final[i], "Final result " + std::to_string(i));
					}
				}
			}
		}

		Platform::Core::deinitialise_GLFW();
	}

	void GraphicsTester::run_performance_tests()
	{
	}
} // namespace Test