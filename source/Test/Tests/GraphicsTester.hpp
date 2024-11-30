#pragma once

#include "Test/TestManager.hpp"

#include "OpenGL/Types.hpp"

namespace Test
{
	class GraphicsTester : public TestManager
	{
	public:
		GraphicsTester() : TestManager(std::string("GRAPHICS")) {}

		void run_unit_tests()        override;
		void run_performance_tests() override;

	private:

		template <typename value_type>
		void test_buffer()
		{
			{SCOPE_SECTION("Byte")
				OpenGL::Buffer byte_buffer = OpenGL::Buffer({OpenGL::BufferStorageFlag::DynamicStorageBit}, sizeof(value_type) * 4);
				const auto arr             = std::array<value_type, 2>{ value_type{2}, value_type{3} };
				const auto min_data        = std::numeric_limits<value_type>::min();
				const auto max_data        = std::numeric_limits<value_type>::max();

				{SCOPE_SECTION("Baseline")
					bool caught_exception = false;
					try { byte_buffer.download_data_array<value_type, 4>(); }
					catch (const std::exception& e) { caught_exception = true; }

					// Buffer = {0, 0, 0, 0}, We expect the buffer to be zeroed out on creation.
					// Cannot download data from an empty buffer so tets for thrown exception instead.
					CHECK_TRUE(caught_exception, "Download data from empty buffer");
					CHECK_EQUAL(byte_buffer.used_capacity(), 0, "Used capacity");
					CHECK_EQUAL(byte_buffer.capacity(), sizeof(value_type) * 4, "Capacity");
				}

				{SCOPE_SECTION("Set index 0")
					byte_buffer.set_data(min_data);
					// Buffer = {0, 0, 0, 0}, Set index 0 to min (0).
					std::array<value_type, 1> expected_result = { min_data };
					auto result = byte_buffer.download_data_array<value_type, 1>();

					CHECK_CONTAINER_EQUAL(result, expected_result, "Check data after setting index 0");
					CHECK_EQUAL(byte_buffer.used_capacity(), sizeof(value_type), "Used capacity");
					CHECK_EQUAL(byte_buffer.capacity(), sizeof(value_type) * 4, "Capacity");
				}
				{SCOPE_SECTION("Set index 1")
					byte_buffer.set_data(max_data);
					// Buffer = {0, 255, 0, 0}, Set index 1 to max.
					std::array<value_type, 2> expected_result = { min_data, max_data };
					auto result = byte_buffer.download_data_array<value_type, 2>();

					CHECK_CONTAINER_EQUAL(result, expected_result, "Check data after setting index 1");
					CHECK_EQUAL(byte_buffer.used_capacity(), sizeof(value_type) * 2, "Used capacity");
					CHECK_EQUAL(byte_buffer.capacity(), sizeof(value_type) * 4, "Capacity");
				}
				{SCOPE_SECTION("Set array to index 2")
					byte_buffer.set_data(arr);
					std::array<value_type, 4> expected_result = { min_data, max_data, arr[0], arr[1] };
					// Buffer = {0, 255, 2, 3}, Set index 2 and 3 to array values.
					auto result = byte_buffer.download_data_array<value_type, 4>();

					CHECK_CONTAINER_EQUAL(result, expected_result, "Check data after setting array");
					CHECK_EQUAL(byte_buffer.used_capacity(), sizeof(value_type) * 4, "Used capacity");
					CHECK_EQUAL(byte_buffer.capacity(), sizeof(value_type) * 4, "Capacity");
				}
				{SCOPE_SECTION("Clear index 1")
					byte_buffer.clear(sizeof(value_type), sizeof(value_type));
					// Clear data not at the end, should not affect used capacity.
					//              V clearing this byte.
					// Buffer = {0, 0, 2, 3}, Clear index 1.
					std::array<value_type, 4> expected_result_non_end_remove = { min_data, value_type{0}, arr[0], arr[1] };
					auto result = byte_buffer.download_data_array<value_type, 4>();

					CHECK_CONTAINER_EQUAL(result, expected_result_non_end_remove, "Clear byte in middle download data");
					CHECK_EQUAL(byte_buffer.used_capacity(), sizeof(value_type) * 4, "Clear byte in middle used capacity");
					CHECK_EQUAL(byte_buffer.capacity(), sizeof(value_type) * 4, "Capacity");
				}
				{SCOPE_SECTION("Clear index 3")
					byte_buffer.clear(3 * sizeof(value_type), sizeof(value_type));
					// Clear data from the end should reduce used capacity.
					//                 C  V Despite clearing off the end, the value is still there in the current implementation.
					// Buffer = {0, 0, 2, 0}, Clear index 3.
					std::array<value_type, 3> expected_result_end = { min_data, value_type{0}, arr[0] };
					auto result = byte_buffer.download_data_array<value_type, 3>();

					CHECK_CONTAINER_EQUAL(result, expected_result_end, "Clear byte at end download data");
					CHECK_EQUAL(byte_buffer.used_capacity(), sizeof(value_type) * 3, "Clear byte at end used capacity");
					CHECK_EQUAL(byte_buffer.capacity(), sizeof(value_type) * 4, "Capacity");
				}
			}
		}
	};
} // namespace Test