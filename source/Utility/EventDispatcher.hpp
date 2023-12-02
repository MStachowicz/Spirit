#pragma once

#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>

namespace Utility
{
	template <typename... EventTypes>
	class EventDispatcher
	{
	public:
		using EventFunction   = std::function<void(EventTypes...)>;
		using EventFunctionID = size_t;

		template <typename T, typename Func>
		EventFunctionID subscribe(T* p_object, Func&& p_event_handler)
		{
			static_assert(std::is_invocable_v<Func, T*, EventTypes...>, "Func must be callable with the specified EventTypes");

			m_handlers.emplace_back(
				++m_last_function_ID,
				std::bind_front(std::forward<Func>(p_event_handler), p_object));
			return m_last_function_ID;
		}

		void unsubscribe(const EventFunctionID& p_function_ID)
		{
			auto it = std::find_if(
				m_handlers.begin(), m_handlers.end(),
				[&p_function_ID](const auto& p_event_handler)
				{
					return p_event_handler.first == p_function_ID;
				});
			if (it == m_handlers.end())
			{
				return;
			}

			*it = std::move(m_handlers.back());
			m_handlers.pop_back();
		}

		void dispatch(EventTypes&&... p_event_types)
		{
			for (const auto& handler : m_handlers)
				handler.second(std::forward<EventTypes>(p_event_types)...);
		}

	private:
		std::vector<std::pair<EventFunctionID, EventFunction>> m_handlers;
		EventFunctionID m_last_function_ID = 0;
	};
} // namespace Utility