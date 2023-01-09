#pragma once

#include <functional>
#include <vector>

namespace Utility
{
    template <typename... EventTypes>
    class EventDispatcher
    {
    public:
        using EventFunction   = std::function<void(const EventTypes&...)>;
        using EventFunctionID = size_t;

        template <typename T, typename Func>
        EventFunctionID subscribe(T* pObject, Func&& pEventHandler)
        {
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

        void dispatch(const EventTypes&... pEvent)
        {
            for (const auto& handler : mHandlers)
            {
                handler.second(pEvent...);
            }
        }

    private:
        std::vector<std::pair<EventFunctionID, EventFunction>> mHandlers;
        EventFunctionID mLastFunctionID = 0;
    };
} // namespace Utility