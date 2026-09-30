#include "GraphicsTester.hpp"

#include "OpenGL/Types.hpp"
#include "OpenGL/Shader.hpp"
#include "OpenGL/GLState.hpp"
#include "OpenGL/DrawCall.hpp"
#include "OpenGL/GridRenderer.hpp"
#include "OpenGL/DebugRenderer.hpp"

#include "Component/ViewInformation.hpp"

#include "Platform/Core.hpp"
#include "Platform/Input.hpp"
#include "Platform/Window.hpp"

#include <glad/glad.h>

#include <algorithm>

namespace Test
{
	void GraphicsTester::run_unit_tests()
	{
		Platform::Core::initialise_directories();
		Platform::Core::initialise_GLFW();
		Platform::Input input   = Platform::Input();
		Platform::Window window = Platform::Window({0.5, 0.5}, input);
		Platform::Core::initialise_OpenGL();


		{SCOPE_SECTION("Buffer")
			{SCOPE_SECTION("Memory handling")
				OpenGL::Buffer buffer = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit}, 1024);

				CHECK_EQUAL(buffer.capacity(), 1024, "Constructor capacity");
				CHECK_EQUAL(buffer.used_capacity(), 0, "Constructor used capacity");

				buffer.reserve(2048);
				CHECK_EQUAL(buffer.capacity(), 2048, "Reserve capacity");
				CHECK_EQUAL(buffer.used_capacity(), 0, "Reserve used capacity");

				buffer.shrink_to_size(1024);
				CHECK_EQUAL(buffer.capacity(), 1024, "Shrink to size capacity");
				CHECK_EQUAL(buffer.used_capacity(), 0, "Shrink to size used capacity");

				buffer.shrink_to_fit();
				CHECK_EQUAL(buffer.capacity(), 0, "Shrink to fit capacity");
				CHECK_EQUAL(buffer.used_capacity(), 0, "Shrink to fit used capacity");

				buffer.reserve(1024);
				CHECK_EQUAL(buffer.capacity(), 1024, "Reserve capacity");
				CHECK_EQUAL(buffer.used_capacity(), 0, "Reserve used capacity");

				test_buffer<std::byte>();
				test_buffer<float>();
				test_buffer<int>();
				test_buffer<uint16_t>();
			}
		}


		{SCOPE_SECTION("Compute")
			{SCOPE_SECTION("Increment")
				std::array<unsigned int, 8> data = { 1, 2, 3, 4, 5, 6, 7, 8 };
				OpenGL::Buffer in_buffer  = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit}, data);
				OpenGL::Buffer out_buffer = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit}, std::array<unsigned int, 8>{0, 0, 0, 0, 0, 0, 0, 0});

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
				std::array<unsigned int, 8> data = { 3, 1, 7, 0, 4, 1, 6, 3 };
				                            //0 // 4  7  5  9
				                            //1 // 11 14
				                            //2 // 25
				OpenGL::Buffer in_buffer = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit}, data);
				if (data.size() % 2 != 0) throw std::runtime_error("Data size must be a power of 2");

				OpenGL::Buffer out_buffer = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit}, std::array<unsigned int, 8>{0, 0, 0, 0, 0, 0, 0, 0});

				OpenGL::Shader shader                            = OpenGL::Shader("global_sum.comp");
				std::array<std::vector<int>, 3> expected_results = {{ {{ 4, 7, 5, 9 }}, {{ 11, 14 }}, { 25 } }};

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

				std::array<unsigned int, 16> data = { 0, 0, 0, 0, 0, 0, 0,    3, 1, 7, 0, 4, 1, 6, 3, 0 }; // last 0 is padding
				if (data.size() % 2 != 0) throw std::runtime_error("Data size must be a power of 2");
				OpenGL::Buffer buff = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit}, data);

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
					OpenGL::Buffer prefix_sum_buffer = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit}, std::vector<unsigned int>(data.size(), 0)); // Must be initialised to 0 for root node to be correct.

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
						prefix_sum_buffer.copy_from_buffer(buff, 0, sizeof(unsigned int) * (data.size() - 1), sizeof(unsigned int));
						auto result = prefix_sum_buffer.download_data<unsigned int>(data.size());

						// Check the result
						for (size_t i = 0; i < expected_final.size(); i++)
							CHECK_EQUAL(result[i], expected_final[i], "Final result " + std::to_string(i));
					}
				}
			}
		}

		// Renders the procedural grid into a 128x128 offscreen framebuffer using the GPU.
		// Checks visibility and adaptive density across perspective/orthographic zoom levels,
		// including positions beyond the old finite grid boundary.
		// Verifies scene depth occludes the grid while grid rendering leaves depth unchanged.
		// Covers horizon, parallel-ray, looking-away and underside views, plus shader reload.
		// These catch shader and render-state regressions that a C++ build cannot detect.
		// Pixel counts allow driver variation; they do not prove visual quality or performance.
		{SCOPE_SECTION("Procedural grid")
			constexpr unsigned int resolution = 128;
			constexpr size_t pixel_count = resolution * resolution;
			OpenGL::FBO target({resolution, resolution}, true, true, false);
			target.set_clear_colour(glm::vec4(0.f, 0.f, 0.f, 1.f));
			OpenGL::GridRenderer grid;
			OpenGL::Buffer view_properties({OpenGL::BufferStorageFlag::DynamicStorageBit}, sizeof(Component::ViewInformation));
			const bool show_origin_arrows = OpenGL::DebugRenderer::m_debug_options.m_show_origin_arrows;
			OpenGL::DebugRenderer::m_debug_options.m_show_origin_arrows = false;

			auto visible_pixels = [](const std::vector<std::byte>& p_pixels)
			{
				size_t count = 0;
				for (size_t offset = 0; offset < p_pixels.size(); offset += 4)
					if (p_pixels[offset] > std::byte{2} || p_pixels[offset + 1] > std::byte{2} || p_pixels[offset + 2] > std::byte{2})
						++count;
				return count;
			};
			auto render_grid = [&](const Component::ViewInformation& p_view, float p_scene_depth = 1.f)
			{
				view_properties.set_data(p_view, 0);
				target.clear();
				glClearTexImage(target.depth_attachment().handle(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, &p_scene_depth);
				grid.draw(target, p_view, view_properties);
				return target.read_pixels();
			};

			Component::ViewInformation view;
			view.m_view_position = glm::vec4(0.f, 100.f, 0.f, 1.f);
			view.m_view = glm::lookAt(glm::vec3(view.m_view_position), glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f));
			view.m_projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 100000.f);
			const auto initial_view = view;
			const auto reference_pixels = render_grid(view);
			const auto reference_count = visible_pixels(reference_pixels);
			CHECK_TRUE(reference_count > pixel_count / 20 && reference_count < pixel_count * 3 / 4, "Orthographic grid is visible and sparse");

			std::array<float, pixel_count> depth_pixels;
			glGetTextureImage(target.depth_attachment().handle(), 0, GL_DEPTH_COMPONENT, GL_FLOAT,
				static_cast<GLsizei>(sizeof(depth_pixels)), depth_pixels.data());
			CHECK_TRUE(std::all_of(depth_pixels.begin(), depth_pixels.end(), [](float p_depth) { return p_depth == 1.f; }), "Grid does not write scene depth");

			for (float extent : {0.1f, 1.f, 100.f, 1000.f, 10000.f})
			{
				view.m_projection = glm::ortho(-extent, extent, -extent, extent, 0.1f, 100000.f);
				const auto count = visible_pixels(render_grid(view));
				const auto difference = std::max(count, reference_count) - std::min(count, reference_count);
				CHECK_TRUE(difference < pixel_count / 20, std::format("Stable grid density at orthographic extent {}", extent));
			}

			view = initial_view;
			const auto plane_clip = view.m_projection * view.m_view * glm::vec4(0.f, 0.f, 0.f, 1.f);
			const float plane_depth = plane_clip.z / plane_clip.w * 0.5f + 0.5f;
			CHECK_EQUAL(visible_pixels(render_grid(view, plane_depth - 0.0001f)), 0, "Geometry in front occludes the grid");
			CHECK_TRUE(visible_pixels(render_grid(view, plane_depth + 0.0001f)) > 0, "Geometry behind does not occlude the grid");

			view.m_view_position = glm::vec4(20000.f, 100.f, 20000.f, 1.f);
			view.m_view = glm::lookAt(glm::vec3(view.m_view_position), glm::vec3(20000.f, 0.f, 20000.f), glm::vec3(0.f, 0.f, -1.f));
			CHECK_TRUE(visible_pixels(render_grid(view)) > pixel_count / 20, "Grid continues beyond the old mesh boundary");

			view.m_projection = glm::perspective(glm::radians(60.f), 1.f, 0.1f, 100000.f);
			for (float distance : {1.f, 10.f, 10000.f})
			{
				view.m_view_position = glm::vec4(distance, distance, distance, 1.f);
				view.m_view = glm::lookAt(glm::vec3(view.m_view_position), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
				const auto count = visible_pixels(render_grid(view));
				CHECK_TRUE(count > pixel_count / 20 && count < pixel_count * 3 / 4, std::format("Readable perspective grid at distance {}", distance));
			}

			view.m_view_position = glm::vec4(0.f, 10.f, 10.f, 1.f);
			view.m_view = glm::lookAt(glm::vec3(view.m_view_position), glm::vec3(0.f, 10.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
			const auto horizon_pixels = render_grid(view);
			CHECK_TRUE(visible_pixels(horizon_pixels) > 0, "Ground remains visible below the horizon");
			bool sky_clear = true;
			for (size_t offset = pixel_count * 2; offset < horizon_pixels.size(); offset += 4)
				sky_clear &= horizon_pixels[offset] == std::byte{0} && horizon_pixels[offset + 1] == std::byte{0} && horizon_pixels[offset + 2] == std::byte{0};
			CHECK_TRUE(sky_clear, "Grid does not render behind the camera or above the horizon");

			view.m_projection = initial_view.m_projection;
			CHECK_EQUAL(visible_pixels(render_grid(view)), 0, "Parallel orthographic rays do not render a grid");
			view.m_view_position = initial_view.m_view_position;
			view.m_view = glm::lookAt(glm::vec3(view.m_view_position), glm::vec3(0.f, 200.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
			CHECK_EQUAL(visible_pixels(render_grid(view)), 0, "Looking away from the plane produces no grid");

			view = initial_view;
			view.m_view_position = glm::vec4(0.f, -100.f, 0.f, 1.f);
			view.m_view = glm::lookAt(glm::vec3(view.m_view_position), glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f));
			CHECK_TRUE(visible_pixels(render_grid(view)) > 0, "Grid is visible from below");

			grid.reload_shaders();
			CHECK_TRUE(visible_pixels(render_grid(initial_view)) > 0, "Grid renders after shader reload");
			OpenGL::DebugRenderer::m_debug_options.m_show_origin_arrows = show_origin_arrows;
		}

		Platform::Core::deinitialise_GLFW();
	}

	void GraphicsTester::run_performance_tests()
	{
	}
} // namespace Test