#pragma once

#include "Logger.hpp"
#include "FunctionTraits.hpp"

#include <type_traits>
#include <stddef.h>
#include <memory>
#include <unordered_set>
#include <optional>

namespace Utility
{
    // Forward declare the ResoureRef class so it can be used by ResourceManager
    template <typename Resource>
    class ResourceRef;

    // ResourceManager is a container for a Resource type.
    // It manages the lifetime of the Resource instances and provides a way to access them via ResourceRef objects.
    template<typename Resource>
    class ResourceManager
    {
        static_assert(std::is_object_v<Resource>, "Non-object types are forbidden. ResourceManager stores the data itself.");

        using RefType = ResourceRef<Resource>;
        friend class RefType;

        // The ResourceData struct is used to store the Resource and the number of references to it.
        // Declaring a struct to contain both avoids the need for manually calculating alignment requirements for the buffer.
        struct ResourceData
        {
            Resource m_resource;
            size_t m_counter;
        };

        // Needs to fit the Resource and ResourceCounter with both aligned to the max alignment.
        static constexpr size_t instance_size    = sizeof(ResourceData);
        static constexpr size_t offsetof_counter = offsetof(ResourceData, m_counter);
        static constexpr size_t initial_capacity = 1u;


        std::size_t m_size;     // The index past the last element in the buffer.
        std::size_t m_capacity; // The number of elements that can be held in currently allocated storage.
        // Buffer for all the instances of ResourceData.
        // Using our own buffer we can control the memory allocation and deallocation.
        // The buffer contains a ResourceData instance at each index unless the index is in m_free_indices.
        std::byte* m_data;
        std::unordered_set<size_t> m_free_indices; // Indices of free elements in the buffer. Memory at these addresses is allocated but not initialised.

    public:
        ResourceManager() noexcept
            : m_size{0u}
            , m_capacity{initial_capacity}
            , m_data{(std::byte*)malloc(m_capacity * instance_size)}
            , m_free_indices{}
        {}
        ~ResourceManager() noexcept
        {// Call the destructor of all the instances then deallocate the memory.
            if (m_data != nullptr)
            {
                clear();
                free(m_data);
            }
        }
        // Move construct a ResourceManager.
        ResourceManager(ResourceManager&& p_other) noexcept
            : m_size{std::move(p_other.m_size)}
            , m_capacity{std::move(p_other.m_capacity)}
            , m_data{std::exchange(p_other.m_data, nullptr)}
            , m_free_indices{std::move(p_other.m_free_indices)}
        {}
        // Move assign a ResourceManager.
        ResourceManager& operator=(ResourceManager&& p_other) noexcept
        {
            if (this != &p_other)
            {
                if (m_data != nullptr)
                {
                    clear();
                    free(m_data);
                }

                m_size         = std::move(p_other.m_size);
                m_capacity     = std::move(p_other.m_capacity);
                m_data         = std::exchange(p_other.m_data, nullptr);
                m_free_indices = std::move(p_other.m_free_indices);
            }

            return *this;
        }
        // Delete the copy constructor and assignment operators.
        ResourceManager(const ResourceManager& p_other)            = delete;
        ResourceManager& operator=(const ResourceManager& p_other) = delete;


        size_t size()     const { return m_size - m_free_indices.size(); }
        size_t capacity() const { return m_capacity; }
        bool empty()      const { return size() == 0; }
        void clear()
        {
            // Call the destructor for all initialised instances of ResourceData.
            for (auto i = 0; i < m_size; i++)
                if (!m_free_indices.contains(i))
                    get_resource(i).~Resource();

            m_size = 0;
        }
        void reserve(std::size_t p_capacity)
        {
            if (p_capacity > m_capacity)
            {
                m_capacity = p_capacity;
                LOG("[ResourceManager] - Capacity changed", alignof(ResourceData));
                std::byte* new_data = (std::byte*)malloc(m_capacity * instance_size);

                // Placement-new move-construct the ResourceData from this into the auxillary store.
                // Then call the destructor on the old instances that were moved.
                for (auto i = 0; i < m_size; i++)
                {
                    if (!m_free_indices.contains(i))
                    {
                        new (&new_data[i * instance_size]) Resource(std::move(get_resource(i)));
                        new (&new_data[(i * instance_size) + offsetof_counter]) size_t(get_counter(i));
                        get_resource(i).~Resource();
                    }
                }

                free(m_data);
                m_data = new_data;
            }
        }

        // Move the Resource into the buffer.
        //@param p_value The Resource to copy into the buffer. Must be move constructible.
        //@return a ResourceRef to the Resource in the buffer.
        [[nodiscard]] RefType insert(Resource&& p_value)
        {
            if (m_free_indices.empty())
            {// Constructing into the end of the buffer.
                if (m_size + 1 > m_capacity)
                    reserve(next_power_of_2(m_capacity));

                auto resource = new (&m_data[m_size * instance_size]) Resource(std::move(p_value));
                auto counter  = new (&m_data[(m_size * instance_size) + offsetof_counter]) size_t(std::move(0));
                auto index    = m_size++;
                return RefType{*resource, *this, index};
            }
            else
            { // Constructing into a gap inside the buffer where a resource was previously erased.
                auto index = *m_free_indices.begin();
                auto resource = new (&m_data[m_size * instance_size]) Resource(std::move(p_value));
                auto counter  = new (&m_data[(m_size * instance_size) + offsetof_counter]) size_t(std::move(0));
                m_free_indices.erase(m_free_indices.begin());
                return RefType{*resource, *this, index};
            }
        }
        // Copy the Resource into the buffer is removed. Prefer to use move insert if possible.
        RefType insert(const Resource& p_value) = delete;


        // Find a Resource in the buffer. If the Resource is not found then create one using construction args and return it.
        // If multiple Resources are found then the first one is returned.
        //@param find_if_func A function that takes a Resource and returns true if it is the Resource we are looking for.
        //@param construction_args The arguments to pass to the Resource constructor if the Resource is not found.
        //@return A valid ResourceRef to the Resource in the buffer.
        template <typename Func, typename... Args>
        [[nodiscard]] RefType get_or_create(const Func&& find_if_func, Args&&... construction_args)
        {
            static_assert(std::is_constructible_v<Resource, Args...>, "construction_args given cannot be used to construct a Resource type");
            static_assert(FunctionTraits<Func>::NumArgs == 1, "find_if_func must take 1 argument");
            static_assert(std::is_same_v<ArgTypeN<Func, 0>, const Resource&>, "Function argument must be a 'const Resource&'");

            for (auto i = 0; i < m_size; i++)
            {
                if (!m_free_indices.contains(i))
                {
                    if (find_if_func(get_resource(i)))
                        return RefType(get_resource(i), *this, i);
                }
            }

            return insert(Resource(std::forward<Args>(construction_args)...));
        }
        template <typename Func>
        void for_each(const Func&& func) const
        {
            static_assert(FunctionTraits<Func>::NumArgs == 1, "func must take 1 argument");
            static_assert(std::is_same_v<ArgTypeN<Func, 0>, const Resource&>, "Function argument must be a 'const Resource&'");

            for (auto i = 0; i < m_size; i++)
                if (!m_free_indices.contains(i))
                    func(get_resource(i));
        }
        template <typename Func>
        void for_each(const Func&& func)
        {
            static_assert(FunctionTraits<Func>::NumArgs == 1, "func must take 1 argument");
            static_assert(std::is_same_v<ArgTypeN<Func, 0>, Resource&>, "Function argument must be a 'Resource&'");

            for (auto i = 0; i < m_size; i++)
                if (!m_free_indices.contains(i))
                    func(get_resource(i));
        }

    private:
        [[nodiscard]] Resource& get_resource(size_t p_index)
        {
           if (p_index >= m_size)
               throw std::out_of_range("Index out of range");
           ASSERT(m_free_indices.contains(p_index) == false, "Trying to access a free p_index!"); // Always

           return *((Resource*)&m_data[p_index * instance_size]);
        }
        [[nodiscard]] size_t& get_counter(size_t p_index)
        {
           if (p_index >= m_size)
               throw std::out_of_range("Index out of range");
           ASSERT(m_free_indices.contains(p_index) == false, "Trying to access a free p_index!"); // Always

           return *((size_t*)&m_data[(p_index * instance_size) + offsetof_counter]);
        }

        // Increment the count for resource at p_index.
        void increment(size_t p_index)
        {
            get_counter(p_index)++;
        }
        // Decrement the count for ResourceData at p_index.
        // If the count reaches 0 then the ResourceData is removed from the manager.
        void decrement(size_t p_index)
        {
            if (p_index >= m_size)                throw std::out_of_range("Index out of range");
            if (m_free_indices.contains(p_index)) throw std::logic_error("Trying to access a free index!");

            if (--get_counter(p_index) == 0)
                erase(p_index);
        }
        void erase(size_t index)
        {
            if (index == m_size - 1)
            {// Erasing the last element in the buffer.
                get_resource(index).~Resource();
                m_size--;
            }
            else
            {// Erasing an element in the middle of the buffer. Leaves gap that can be used constructing new Resource.
                get_resource(index).~Resource();
                m_free_indices.insert(index);
            }
        }

    	// Returns the next power of 2 larger than p_val
		static size_t next_power_of_2(const size_t& p_val)
		{
			size_t power = 1;
			while (power <= p_val)
				power *= 2;

			return power;
		}
    };

    template<typename Resource>
    class ResourceRef
    {
        using Manager = ResourceManager<Resource>;

        Resource* m_data;   // A non-owning pointer to the resource in the ResourceManager's storage.
        Manager* m_manager; // A non-owning pointer to the ResourceManager that owns the resource.
        std::optional<size_t> m_index;     // The index of the ResourceData in the ResourceManager, allows us to avoid linear searching the Manager buffer.

        // The ResourceManager is a friend so it can access the only valid constructor (private).
        friend class Manager;
        ResourceRef(Resource& p_data, Manager& p_manager, size_t p_index) noexcept : m_data(&p_data), m_manager(&p_manager), m_index(p_index)
        {
            p_manager.increment(*m_index);
        }
    public:
        // Default construct an invalid ResourceRef. Equivalent to constructing a nullopt optional in std.
        ResourceRef()  noexcept : m_index(std::nullopt), m_data(nullptr), m_manager{nullptr} {}        // On destory decrement the referece count for the manager.
        ~ResourceRef() noexcept
        {
            if (is_valid())
                m_manager->decrement(*m_index);
        }

         // On copy construct, copy the resource ptr and manager ptr and increment the count.
        ResourceRef(const ResourceRef& p_other) noexcept
            : m_data{p_other.m_data}
            , m_manager{p_other.m_manager}
            , m_index{p_other.m_index}
        {
            if (is_valid())
                m_manager->increment(*m_index);
        }
        // On copy assigment, decrement the count for the current resource and assign this ResourceRef p_other data.
        ResourceRef& operator=(const ResourceRef& p_other) noexcept
        {
            // If the resource is the same one managed by other, we can safely skip the decrement and increments since the net ResourceRef change will be 0
            if (this != &p_other)
            {
                if (is_valid())
                    m_manager->decrement(*m_index);

                m_data    = p_other.m_data;
                m_manager = p_other.m_manager;
                m_index   = p_other.m_index;

                if (is_valid())
                    m_manager->increment(*m_index);
            }
            return *this;
        }
        // On move construct, move the resource ptr and manager ptr and index. Leave the old ResourceRef in an invalid state.
        ResourceRef(ResourceRef&& p_other) noexcept
            : m_data{std::exchange(p_other.m_data, nullptr)}
            , m_manager{std::exchange(p_other.m_manager, nullptr)}
            , m_index{std::exchange(p_other.m_index, std::nullopt)}
        {}
        // On move assignment, decrement the count for the current resource and steal p_other's data.
        ResourceRef& operator=(ResourceRef&& p_other) noexcept
        {
            if (this != &p_other)
            {
                if (is_valid())
                    m_manager->decrement(*m_index);

                m_data    = std::exchange(p_other.m_data, nullptr);
                m_manager = std::exchange(p_other.m_manager, nullptr);
                m_index   = std::exchange(p_other.m_index, std::nullopt);
            }
            return *this;
        }

        constexpr const Resource* operator->() const noexcept   { return m_data;  };
        constexpr Resource* operator->() noexcept               { return m_data;  };
        constexpr const Resource& operator*() const& noexcept   { return *m_data; };
        constexpr Resource& operator*() & noexcept              { return *m_data; };
        constexpr const Resource&& operator*() const&& noexcept { return *m_data; };
        constexpr Resource&& operator*() && noexcept            { return *m_data; };
        constexpr bool is_valid() const noexcept                { return m_data != nullptr; };
        constexpr explicit operator bool() const noexcept       { return is_valid(); };
    };
} // namespace Utility