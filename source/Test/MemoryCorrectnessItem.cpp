#include "MemoryCorrectnessItem.hpp"

#include "Utility/Logger.hpp"

#include <format>

DISABLE_WARNING_PUSH
DISABLE_WARNING_UNINITIALIZED // Prevent warnings about usage of uninitialized m_memory_initialisation_token

namespace Test
{
	// Constructor
	MemoryCorrectnessItem::MemoryCorrectnessItem()
		: m_ID(construct_memory_correctness_ID())
		, m_member(std::nullopt)
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Constructing {}", to_string());

		//if (m_memory_initialisation_token == 0x2c1dd27f0d59cf3e && m_status != MemoryStatus::Deleted)
		//{
		//	LOG_ERROR("[MEMCORRECTNESS][ERROR] Construction in already initialized memory at {}", to_string_and_memory_status());
		//	s_error_count += 1;
		//}

		status() = MemoryStatus::Constructed;
		//m_memory_initialisation_token = 0x2c1dd27f0d59cf3e;
		s_constructed_count += 1;
	}

	// Destructor
	MemoryCorrectnessItem::~MemoryCorrectnessItem()
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Deleting {}", to_string());

		//if (m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		//{
		//	LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while deleting at {}", to_string());
		//	s_error_count += 1;
		//}
		if (status() == MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Double delete detected at {}", to_string());
			s_error_count += 1;
		}

		m_member.reset();
		status() = MemoryStatus::Deleted;
		s_destroy_count += 1;
	}

	// Copy Constructor
	MemoryCorrectnessItem::MemoryCorrectnessItem(const MemoryCorrectnessItem& p_other)
		: m_ID(construct_memory_correctness_ID())
		, m_member(p_other.m_member)
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Copy constructing {} from {}", to_string(), p_other.to_string());

		//if (p_other.m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		//{
		//	LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while copy constructing from {}", p_other.to_string());
		//	s_error_count += 1;
		//}
		if (p_other.status() == MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Copy constructing from deleted memory at {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.status() == MemoryStatus::MovedFrom)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Copy constructing from moved-from memory at {}", p_other.to_string());
			s_error_count += 1;
		}

		//if (m_memory_initialisation_token == 0x2c1dd27f0d59cf3e && status() != MemoryStatus::Deleted)
		//{
		//	LOG_ERROR("[MEMCORRECTNESS][ERROR] Copy construction in already initialized memory at {}", to_string_and_memory_status());
		//	s_error_count += 1;
		//}

		status() = MemoryStatus::Constructed;
		//m_memory_initialisation_token = 0x2c1dd27f0d59cf3e;
		s_copy_construct_count += 1;
	}

	// Move Constructor
	MemoryCorrectnessItem::MemoryCorrectnessItem(MemoryCorrectnessItem&& p_other)
		: m_ID(construct_memory_correctness_ID())
		, m_member(std::move(p_other.m_member))
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Move constructing {} from {}", to_string(), p_other.to_string());

		//if (p_other.m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		//{
		//	LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while move constructing from {}", p_other.to_string());
		//	s_error_count += 1;
		//}
		if (p_other.status() == MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Move constructing from deleted memory at {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.status() == MemoryStatus::MovedFrom)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Move constructing from moved-from memory at {}", p_other.to_string());
			s_error_count += 1;
		}

		//if (m_memory_initialisation_token == 0x2c1dd27f0d59cf3e && status() != MemoryStatus::Deleted)
		//{
		//	LOG_ERROR("[MEMCORRECTNESS][ERROR] Move construction in already initialized memory at {}", to_string_and_memory_status());
		//	s_error_count += 1;
		//}

		p_other.status()               = MemoryStatus::MovedFrom;
		status()                       = MemoryStatus::Constructed;
		//m_memory_initialisation_token = 0x2c1dd27f0d59cf3e;
		s_move_construct_count         += 1;
	}

	// Copy Assignment
	MemoryCorrectnessItem& MemoryCorrectnessItem::operator=(const MemoryCorrectnessItem& p_other)
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Copy assigning {} from {}", to_string(), p_other.to_string());

		//if (p_other.m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		//{
		//	LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while copy assigning from {}", p_other.to_string());
		//	s_error_count += 1;
		//}
		if (p_other.status() == MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Copy assigning from deleted memory at {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.status() == MemoryStatus::MovedFrom)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Copy assigning from moved-from memory at {}", p_other.to_string());
			s_error_count += 1;
		}

		//if (m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		//{
		//	LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while copy assigning to {}", to_string());
		//	s_error_count += 1;
		//}

		m_member = p_other.m_member;
		s_copy_assign_count += 1;
		return *this;
	}

	// Move Assignment
	MemoryCorrectnessItem& MemoryCorrectnessItem::operator=(MemoryCorrectnessItem&& p_other)
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Move assigning {} from {}", to_string(), p_other.to_string());

		//if (p_other.m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		//{
		//	LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while move assigning from {}", p_other.to_string());
		//	s_error_count += 1;
		//}
		if (p_other.status() == MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Move assigning from deleted memory at {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.status() == MemoryStatus::MovedFrom)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Move assigning from moved-from memory at {}", p_other.to_string());
			s_error_count += 1;
		}

		//if (m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		//{
		//	LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while move assigning to {}", to_string());
		//	s_error_count += 1;
		//}

		p_other.status() = MemoryStatus::MovedFrom;
		m_member         = std::move(p_other.m_member);
		s_move_assign_count += 1;
		return *this;
	}

	MemoryCorrectnessItem::MemoryStatus& MemoryCorrectnessItem::status() const
	{
		return s_memory_statuses[m_ID];
	}
	std::string MemoryCorrectnessItem::to_string() const
	{
		return std::format("ID: {} ({})", m_ID, (void*)(this));
	}
	std::string MemoryCorrectnessItem::to_string_and_memory_status() const
	{
		switch (status())
		{
			case MemoryStatus::Uninitialized: return std::format("ID: {} ({}) - Memory status was: Uninitialized", m_ID, (void*)(this));
			case MemoryStatus::Constructed:   return std::format("ID: {} ({}) - Memory status was: Constructed", m_ID, (void*)(this));
			case MemoryStatus::MovedFrom:     return std::format("ID: {} ({}) - Memory status was: MovedFrom", m_ID, (void*)(this));
			case MemoryStatus::Deleted:       return std::format("ID: {} ({}) - Memory status was: Deleted", m_ID, (void*)(this));
			default:                          return std::format("ID: {} ({}) - Memory status was: Unknown", m_ID, (void*)(this));
		}
	}

	size_t MemoryCorrectnessItem::construct_memory_correctness_ID()
	{
		// If the vector size is not equal to the ID, something went wrong.
		//ASSERT_THROW(s_memory_statuses.size() != p_ID, "[MEMCORRECTNESS][ERROR] Constructing memory status for ID {} while the vector size is {}", p_ID, s_memory_statuses.size());
		s_memory_statuses.push_back(MemoryStatus::Uninitialized);
		return s_memory_statuses.size() - 1;
	}

	void MemoryCorrectnessItem::reset()
	{
		s_constructed_count    = 0;
		s_destroy_count        = 0;
		s_copy_construct_count = 0;
		s_move_construct_count = 0;
		s_copy_assign_count    = 0;
		s_move_assign_count    = 0;
		s_error_count          = 0;

		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("RESET MemoryCorrectnessItem");
	}
} // namespace Test
DISABLE_WARNING_POP