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
        EventFunctionID subscribe(T* pObject, Func&& pEventHandler)
        {
            static_assert(std::is_invocable_v<Func, T*, EventTypes...>, "Func must be callable with the specified EventTypes");

            mHandlers.emplace_back(
                ++mLastFunctionID,
                std::bind_front(std::forward<Func>(pEventHandler), pObject));
            return mLastFunctionID;
        }

        void unsubscribe(const EventFunctionID& pFunctionID)
        {
            auto it = std::find_if(
                mHandlers.begin(), mHandlers.end(),
                [&pFunctionID](const auto& pEventHandler)
                {
                    return pEventHandler.first == pFunctionID;
                });
            if (it == mHandlers.end())
            {
                return;
            }

            *it = std::move(mHandlers.back());
            mHandlers.pop_back();
        }

        void dispatch(EventTypes&&... pEventTypes)
        {
            for (const auto& handler : mHandlers)
                handler.second(std::forward<EventTypes>(pEventTypes)...);
        }

    private:
        std::vector<std::pair<EventFunctionID, EventFunction>> mHandlers;
        EventFunctionID mLastFunctionID = 0;
    };
} // namespace Utility