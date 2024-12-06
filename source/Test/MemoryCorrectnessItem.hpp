#pragma once

#include <optional>
#include <string>
#include <vector>

namespace Test
{
	class MemoryCorrectnessItem
	{
		constexpr static bool LOG_MEM_CORRECTNESS_EVENTS = false;

		enum class MemoryStatus
		{
			Uninitialized = 0,
			Constructed   = 1,
			MovedFrom     = 2,
			Deleted       = 3
		};

		inline static size_t s_constructed_count    = 0;
		inline static size_t s_destroy_count        = 0;
		inline static size_t s_copy_construct_count = 0;
		inline static size_t s_move_construct_count = 0;
		inline static size_t s_copy_assign_count    = 0;
		inline static size_t s_move_assign_count    = 0;
		inline static size_t s_error_count          = 0;
		inline static size_t s_instance_ID          = 0; // Unique ID per instance of MemoryCorrectnessItem
		// Per MemoryCorrectnessItem instance, stores the status of the memory and the. Index is the instance m_ID.
		inline static std::vector<MemoryStatus> s_memory_statuses = {};
		static size_t construct_memory_correctness_ID();


		size_t m_ID; // Unique ID of a constructed MemoryCorrectnessItem instance.
		MemoryStatus& status() const;

		std::string to_string() const;
		std::string to_string_and_memory_status() const;
	public:
		constexpr static size_t Persistent_ID = 0; // Required for ECSTester
		std::optional<int> m_member; // A faux member to emulate a resource storage of the object.

		static void reset();
		static size_t count_alive()
		{
			return s_constructed_count + s_copy_construct_count + s_move_construct_count - s_destroy_count;
		}
		static size_t count_errors() { return s_error_count; }
		static size_t count_copies() { return s_copy_construct_count + s_copy_assign_count; }
		static size_t count_moves()  { return s_move_construct_count + s_move_assign_count; }

		MemoryCorrectnessItem();
		~MemoryCorrectnessItem();
		MemoryCorrectnessItem(const MemoryCorrectnessItem& p_other);
		MemoryCorrectnessItem(MemoryCorrectnessItem&& p_other);
		MemoryCorrectnessItem& operator=(const MemoryCorrectnessItem& p_other);
		MemoryCorrectnessItem& operator=(MemoryCorrectnessItem&& p_other);
	};
} // namespace Test