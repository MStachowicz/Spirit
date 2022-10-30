#pragma once

#include <functional>
#include <vector>

namespace Utility
{
    // Supplied a list of arguments, mediates functions subscribing to and receiving callbacks when a specific event occurs.
    template<class... Args>
    class EventDispatcher
    {
    public:
        void Subscribe(const std::function<void(Args&&... pArgs)>& pSubscriber)
        {
            mSubscribers.push_back(pSubscriber);
        }
        void Dispatch(Args&&... pArgs)
        {
            for(const auto& subscriber : mSubscribers)
                subscriber(std::forward<Args>(pArgs)...);
        }

    private:
        std::vector<std::function<void(Args&&...)>> mSubscribers;
    };
}