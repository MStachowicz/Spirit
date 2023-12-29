#pragma once

#include <string>
#include <optional>

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
		inline static size_t s_instance_ID             = 0; // Unique ID per instance of MemoryCorrectnessItem

		size_t m_ID; // Unique ID of a constructed MemoryCorrectnessItem instance.
		// Padding to push the memory_status back a little
		// Without this, some tests seem to generate false positive errors, which could be due to the memory at the start
		// of the object being reused for something else after deletion, changing the memory status while leaving the
		// token intact.
		std::byte m_padding[16];
		volatile MemoryStatus m_status;
		volatile size_t m_memory_initialisation_token;

		std::string to_string() const;
		std::string get_memory_status() const;
		std::string to_string_and_memory_status() const;
	public:
		std::optional<int> m_member; // A faux member to emulate a resource storage of the object.

		size_t ID() const { return m_ID; }
		static void reset();
		static size_t count_alive()
		{
			return s_constructed_count + s_copy_construct_count + s_move_construct_count - s_destroy_count;
		}
		static size_t count_errors()
		{
			return s_error_count;
		}

		MemoryCorrectnessItem();
		~MemoryCorrectnessItem();
		MemoryCorrectnessItem(const MemoryCorrectnessItem& p_other);
		MemoryCorrectnessItem(MemoryCorrectnessItem&& p_other);
		MemoryCorrectnessItem& operator=(const MemoryCorrectnessItem& p_other);
		MemoryCorrectnessItem& operator=(MemoryCorrectnessItem&& p_other);
	};
} // namespace Test