#include "MemoryCorrectnessItem.hpp"

#include "Utility/Logger.hpp"

#include <format>
#include <iostream>

DISABLE_WARNING_PUSH
DISABLE_WARNING_UNINITIALIZED // Prevent warnings about usage of uninitialized m_memory_initialisation_token

namespace Test
{
	MemoryCorrectnessItem::MemoryCorrectnessItem()
		: m_ID(s_instance_ID++)
		, m_member(std::nullopt)
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Constructing {}", to_string());

		if (m_memory_initialisation_token == 0x2c1dd27f0d59cf3e && m_status != MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Construction in already initialized memory at {}", to_string_and_memory_status());
			s_error_count += 1;
		}

		m_status                      = MemoryStatus::Constructed;
		m_memory_initialisation_token = 0x2c1dd27f0d59cf3e;
		s_constructed_count += 1;
	}
	MemoryCorrectnessItem::~MemoryCorrectnessItem()
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Deleting {}", to_string());

		if (m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while deleting at {}", to_string());
			s_error_count += 1;
		}
		if (m_status == MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Double delete detected at {}", to_string());
			s_error_count += 1;
		}

		m_member.reset();
		m_status = MemoryStatus::Deleted;
		s_destroy_count += 1;
	}

	MemoryCorrectnessItem::MemoryCorrectnessItem(const MemoryCorrectnessItem& p_other)
		: m_ID(s_instance_ID++)
		, m_member(p_other.m_member)
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Copy constructing {} from {}", to_string(), p_other.to_string());

		if (p_other.m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while copy constructing from {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.m_status == MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Copy constructing from deleted memory at {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.m_status == MemoryStatus::MovedFrom)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Copy constructing from moved-from memory at {}", p_other.to_string());
			s_error_count += 1;
		}

		if (m_memory_initialisation_token == 0x2c1dd27f0d59cf3e && m_status != MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Copy construction in already initialized memory at {}", to_string_and_memory_status());
			s_error_count += 1;
		}

		m_status = MemoryStatus::Constructed;
		m_memory_initialisation_token = 0x2c1dd27f0d59cf3e;
		s_copy_construct_count += 1;
	}

	MemoryCorrectnessItem::MemoryCorrectnessItem(MemoryCorrectnessItem&& p_other)
		: m_ID(s_instance_ID++)
		, m_member(std::move(p_other.m_member))
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Move constructing {} from {}", to_string(), p_other.to_string());

		if (p_other.m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while move constructing from {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.m_status == MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Move constructing from deleted memory at {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.m_status == MemoryStatus::MovedFrom)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Move constructing from moved-from memory at {}", p_other.to_string());
			s_error_count += 1;
		}

		if (m_memory_initialisation_token == 0x2c1dd27f0d59cf3e && m_status != MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Move construction in already initialized memory at {}", to_string_and_memory_status());
			s_error_count += 1;
		}

		p_other.m_status               = MemoryStatus::MovedFrom;
		m_status                       = MemoryStatus::Constructed;
		m_memory_initialisation_token  = 0x2c1dd27f0d59cf3e;
		s_move_construct_count         += 1;
	}

	MemoryCorrectnessItem& MemoryCorrectnessItem::operator=(const MemoryCorrectnessItem& p_other)
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Copy assigning {} from {}", to_string(), p_other.to_string());

		if (p_other.m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while copy assigning from {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.m_status == MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Copy assigning from deleted memory at {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.m_status == MemoryStatus::MovedFrom)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Copy assigning from moved-from memory at {}", p_other.to_string());
			s_error_count += 1;
		}

		if (m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while copy assigning to {}", to_string());
			s_error_count += 1;
		}

		m_member = p_other.m_member;
		s_copy_assign_count += 1;
		return *this;
	}

	MemoryCorrectnessItem& MemoryCorrectnessItem::operator=(MemoryCorrectnessItem&& p_other)
	{
		if constexpr (LOG_MEM_CORRECTNESS_EVENTS)
			LOG("Move assigning {} from {}", to_string(), p_other.to_string());

		if (p_other.m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while move assigning from {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.m_status == MemoryStatus::Deleted)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Move assigning from deleted memory at {}", p_other.to_string());
			s_error_count += 1;
		}
		if (p_other.m_status == MemoryStatus::MovedFrom)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Move assigning from moved-from memory at {}", p_other.to_string());
			s_error_count += 1;
		}

		if (m_memory_initialisation_token != 0x2c1dd27f0d59cf3e)
		{
			LOG_ERROR("[MEMCORRECTNESS][ERROR] Use of uninitialized memory while move assigning to {}", to_string());
			s_error_count += 1;
		}

		p_other.m_status   = MemoryStatus::MovedFrom;
		m_member         = std::move(p_other.m_member);
		s_move_assign_count += 1;
		return *this;
	}

	std::string MemoryCorrectnessItem::get_memory_status() const
	{
		const static char* memoryStatusNames[4] = {"Uninitialized", "Constructed", "MovedFrom", "Deleted"};

		if ((int)m_status >= 0 && (int)m_status <= 3)
			return std::format("Memory status was: {}", memoryStatusNames[(int)m_status]);
		else
			return std::format("Memory status was: {}", (int)m_status);
	}
	std::string MemoryCorrectnessItem::to_string() const
	{
		return std::format("ID: {} ({})", m_ID, (void*)(this));
	}
	std::string MemoryCorrectnessItem::to_string_and_memory_status() const
	{
		return std::format("{} - {}", to_string(), get_memory_status());
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