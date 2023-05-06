#pragma once

#include "EventDispatcher.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace Utility
{
    template <typename... Args>
    struct ArgTypes
    {};

    template <typename T, typename... Args>
    struct ArgTypes<T, Args...>
    {
        using Head = T;
        using Tail = ArgTypes<Args...>;
    };

    template <typename... Args>
    using ExtractArgTypes = ArgTypes<Args...>;

    template <typename Func>
    struct FunctionTraits : public FunctionTraits<decltype(&Func::operator())>
    {};

    template <typename ClassType, typename ReturnType, typename... Args>
    struct FunctionTraits<ReturnType (ClassType::*)(Args...) const>
    {
        using Return                         = ReturnType;
        using ArgsTuple                      = std::tuple<Args...>;
        using ArgTypes                       = ExtractArgTypes<Args...>;
        static constexpr std::size_t NumArgs = std::tuple_size_v<ArgsTuple>;
    };

    template <typename ReturnType>
    struct FunctionTraits<ReturnType()>
    {
        using Return                         = ReturnType;
        using ArgsTuple                      = std::tuple<>;
        using ArgTypes                       = ExtractArgTypes<>;
        static constexpr std::size_t NumArgs = 0;
    };

    template <typename Func>
    using ReturnType = typename FunctionTraits<Func>::Return;

    template <typename Func>
    using ArgsTuple = typename FunctionTraits<Func>::ArgsTuple;

    template <typename Func, std::size_t N>
    using ArgTypeN = typename std::tuple_element_t<N, ArgsTuple<Func>>;

    // Forward declare the ResoureRef class so it can be used by ResourceManager
    template <typename Resource>
    class ResourceRef;

    // ResourceManager handles the construction and destruction of Resource types.
    // This manager acts as a factory and returns shared ownership ResourceRef types that can be safely used with RAII.
    // The get and getOrCreate functions allow you to lookup or lookup and create (if not found) instances of Resource wrapped by ResourceRef.
    template <typename Resource>
    class ResourceManager
    {
        static_assert(!std::is_pointer_v<Resource> && !std::is_reference_v<Resource>, "Resource must not be a pointer or reference type. ResourceManager manages the memory via the ReferenceRef counters");
        friend class ResourceRef<Resource>; // make ResourceRef a friend class - only ResourceRef can call increment and decrement count.

        // A collection of Resource objects, which also keeps track of their reference counts via calls to increment and decrement count.
        std::vector<std::pair<std::unique_ptr<Resource>, size_t>> mResourcesAndCounts;

        // Increment the count for pResource.
        void incrementCount(Resource* pResource)
        {
            for (auto& resourceAndCount : mResourcesAndCounts)
            {
                if (resourceAndCount.first.get() == pResource)
                {
                    ++resourceAndCount.second;
                    return;
                }
            }
        }
        // Decrement the count for pResource, if reached 0 delete the underlying resource using swap and pop idiom.
        void decrementCount(Resource* pResource)
        {
            for (size_t i = 0; i < mResourcesAndCounts.size(); ++i)
            {
                if (mResourcesAndCounts[i].first.get() == pResource)
                {
                    --mResourcesAndCounts[i].second;
                    if (mResourcesAndCounts[i].second == 0)
                    {
                        mResourcesAndCounts[i] = std::move(mResourcesAndCounts.back());
                        mResourcesAndCounts.pop_back();
                    }
                    return;
                }
            }
        }

    public:
        ResourceManager()  = default;
        ~ResourceManager() = default;
        // Delete the copy constructor and assignment operators.
        ResourceManager(const ResourceManager& other)            = delete;
        ResourceManager& operator=(const ResourceManager& other) = delete;

        // Move constructor
        ResourceManager(ResourceManager&& other) noexcept
            : mResourcesAndCounts(std::move(other.mResourcesAndCounts))
        {}
        // Move assignment operator
        ResourceManager& operator=(ResourceManager&& other) noexcept
        {
            if (this != &other)
                mResourcesAndCounts = std::move(other.mResourcesAndCounts);

            return *this;
        }

        // Try to get a ResourceRef using pFindIfFunction, returns a std::nullopt if a ResourceRef was not found.
        template <typename Func>
        std::optional<ResourceRef<Resource>> get(const Func&& pFindIfFunction)
        {
            static_assert(std::is_same_v<ReturnType<Func>, bool>, "Function must return bool");
            static_assert(FunctionTraits<Func>::NumArgs == 1, "Function must take 1 argument");
            static_assert(std::is_same_v<ArgTypeN<Func, 0>, const Resource&>, "Function argument must be a 'const Resource&'");

            if (mResourcesAndCounts.empty())
                return std::nullopt;

            for (auto& [resource, count] : mResourcesAndCounts)
            {
                if (pFindIfFunction(*resource))
                {
                    return ResourceRef<Resource>(resource.get(), this);
                }
            }

            return std::nullopt;
        }

        // Force create a Resource using pConstructionArgs.
        // This function doesn't check if the Resource already exists therefore can lead to duplicate Resources.
        // Prefer to use getOrCreate to search the already created Resource objects before adding a new one.
        template <typename... Args>
        ResourceRef<Resource> create(Args&&... pConstructionArgs)
        {
            static_assert(std::is_constructible_v<Resource, Args...>, "Args given cannot be used to construct a Resource type");

            mResourcesAndCounts.emplace_back(std::make_unique<Resource>(std::forward<Args>(pConstructionArgs)...), 0);
            return ResourceRef<Resource>(mResourcesAndCounts.back().first.get(), this);
        }

        // Try to get a ResourceRef using pFindIfFunction, if one is not found, uses pConstructionArgs to construct a new Resource and return a ResourceRef to it.
        template <typename Func, typename... Args>
        ResourceRef<Resource> getOrCreate(const Func&& pFindIfFunction, Args&&... pConstructionArgs)
        {
            static_assert(std::is_constructible_v<Resource, Args...>, "Args given cannot be used to construct a Resource type");

            if (auto resourceRef = get(std::forward<const Func>(pFindIfFunction)))
            {
                return *resourceRef;
            }
            else
            {
                mResourcesAndCounts.emplace_back(std::make_unique<Resource>(std::forward<Args>(pConstructionArgs)...), 0);
                return ResourceRef<Resource>(mResourcesAndCounts.back().first.get(), this);
            }
        }

        // Iterate over every Resource in the Manager and call pFunction on it
        template <typename Func>
        void forEach(const Func&& pFunction) const
        {
            static_assert(FunctionTraits<Func>::NumArgs == 1, "Function must take 1 argument");
            static_assert(std::is_same_v<ArgTypeN<Func, 0>, const Resource&>, "Function argument must be a 'const Resource&'");

            for (auto& [resource, count] : mResourcesAndCounts)
            {
                pFunction(*resource);
            }
        }

        // Iterate over every Resource in the Manager and call p_function on it.
        //@param p_function Function which will be called on every element in the manager. It must be invocable with a type Resource.
        template <typename Func>
        requires std::invocable<Func, Resource>
        void for_each(const Func&& p_function)
        {
            for (auto& [resource, count] : mResourcesAndCounts)
                p_function(*resource);
        }
    };

    // A wrapper around a pointer to a Resource. ResourceRef is guaranteed to always point at a valid Resource* and can be directly dereferenced and used as though it was a type Resource*.
    // The memory and ownership of Resource objects is handled by a corresponding ResourceManager<Resource>.
    // ResourceRef objects can be safely copied moved but cannot be themselves constructed other than by calling ResourceManager::getOrCreate.
    template <typename Resource>
    class ResourceRef
    {
        static_assert(!std::is_pointer_v<Resource> && !std::is_reference_v<Resource>, "Resource must not be a pointer or reference type. ResourceManager manages the memory via the ReferenceRef counters");
        friend class ResourceManager<Resource>;

        Resource* mResource;
        ResourceManager<Resource>* mResourceManager;

        // The only constructor of ResourceRef, accessible only to ResourceManager.
        ResourceRef(Resource* pResource, ResourceManager<Resource>* pResourceManager) noexcept
            : mResource(pResource)
            , mResourceManager(pResourceManager)
        {
            mResourceManager->incrementCount(mResource);
        }

    public:
        // On destory decrement the referece count for the manager.
        ~ResourceRef() noexcept
        {
            if (mResourceManager)
                mResourceManager->decrementCount(mResource);
        }
        // On copy construct, copy the resource ptr and manager ptr and increment the count.
        ResourceRef(const ResourceRef& pOther) noexcept
            : mResource(pOther.mResource)
            , mResourceManager(pOther.mResourceManager)
        {
            mResourceManager->incrementCount(mResource);
        }

        // On copy assigment, decrement the count for the current resource and assign this ResourceRef pOther data.
        ResourceRef& operator=(const ResourceRef& pOther) noexcept
        {
            // If the resource is the same one managed by other, we can safely skip the decrement and increments since the net ResourceRef change will be 0
            if (this != &pOther && mResource != pOther.mResource)
            {
                mResourceManager->decrementCount(mResource);
                mResource        = pOther.mResource;
                mResourceManager = pOther.mResourceManager;
                mResourceManager->incrementCount(mResource);
            }
            return *this;
        }

        ResourceRef(ResourceRef&& pOther) noexcept
            : mResource(pOther.mResource)
            , mResourceManager(pOther.mResourceManager)
        {
            pOther.mResource        = nullptr;
            pOther.mResourceManager = nullptr;
        }
        ResourceRef& operator=(ResourceRef&& pOther) noexcept
        {
            // If the resource is the same one managed by other, we can safely skip the decrement and increments since the net ResourceRef change will be 0
            if (this != &pOther && mResource != pOther.mResource)
            {
                mResourceManager->decrementCount(mResource);
                mResource               = pOther.mResource;
                mResourceManager        = pOther.mResourceManager;
                pOther.mResource        = nullptr;
                pOther.mResourceManager = nullptr;
            }
            return *this;
        }

        Resource& operator*() { return *mResource; }
        const Resource& operator*() const { return *mResource; }
        Resource* const operator->() { return mResource; }
        const Resource* const operator->() const { return mResource; }
    };
} // namespace Utility