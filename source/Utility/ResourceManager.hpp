#pragma once

#include "Logger.hpp"
#include "FunctionTraits.hpp"

#include <stddef.h>
#include <optional>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Utility
{
	constexpr bool LOG_REF_EVENTS = false;

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
		friend RefType;

		struct ResourceData
		{
			ResourceData(Resource&& p_resource, size_t p_count) noexcept : m_resource(std::move(p_resource)), m_count(p_count) {}
			~ResourceData() noexcept = default;
			ResourceData& operator=(ResourceData&& p_other) noexcept = default;
			ResourceData(ResourceData&& p_other) noexcept            = default;
			ResourceData& operator=(const ResourceData& p_other)     = delete;
			ResourceData(const ResourceData& p_other)                = delete;

			Resource m_resource;
			size_t m_count;
		};

		std::vector<std::optional<ResourceData>> m_resources;
		std::unordered_set<size_t> m_free_indices; // Indices of free elements in the buffer. Memory at these addresses is allocated but not initialised.

	public:
		ResourceManager() noexcept                                     = default;
		~ResourceManager() noexcept                                    = default;
		ResourceManager& operator=(ResourceManager&& p_other) noexcept = default;
		ResourceManager(ResourceManager&& p_other) noexcept            = default;

		// Delete the copy constructor and assignment operators.
		ResourceManager(const ResourceManager& p_other)            = delete;
		ResourceManager& operator=(const ResourceManager& p_other) = delete;

		size_t size()     const { return m_resources.size() - m_free_indices.size(); }
		size_t capacity() const { return m_resources.capacity(); }
		bool empty()      const { return size() == 0; }
		void clear()
		{
			// TODO: ResourceRefs given out should be invalidated when a buffer is cleared.
			// Call the destructor for all initialised instances of ResourceData.
			for (size_t i = 0; i < m_resources.size(); i++)
				if (!m_free_indices.contains(i))
					m_resources[i].reset();

			if constexpr (LOG_REF_EVENTS) LOG("[ResourceManager] Cleared all resources");
		}
		void reserve(std::size_t p_capacity)
		{
			m_resources.reserve(p_capacity);
		}

		// Move the Resource into the manager.
		//@param p_value The Resource to move into the manager. Must be move constructible.
		//@return a ResourceRef to the Resource owned by the manager.
		[[nodiscard]] RefType insert(Resource&& p_value)
		{
			if (m_free_indices.empty())
			{// Constructing into the end of the buffer.
				m_resources.emplace_back(ResourceData(std::move(p_value), 0));
				if constexpr (LOG_REF_EVENTS) LOG("[ResourceManager] Inserting ResourceRef at end index {}",  m_resources.size() - 1);
				return RefType{*this, m_resources.size() - 1};
			}
			else
			{ // Constructing into a gap inside the buffer where a resource was previously erased.
				auto index = *m_free_indices.begin();
				m_resources.emplace(m_resources.begin() + index, ResourceData(std::move(p_value), 0));
				m_free_indices.erase(m_free_indices.begin());
				if constexpr (LOG_REF_EVENTS) LOG("[ResourceManager] Inserting ResourceRef into gap at index {}", index);
				return RefType{*this, index};
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

			for (size_t i = 0; i < m_resources.size(); i++)
			{
				if (!m_free_indices.contains(i))
				{
					if (find_if_func(get_resource(i)))
						return RefType(*this, i);
				}
			}

			return insert(Resource(std::forward<Args>(construction_args)...));
		}
		template <typename Func>
		void for_each(const Func&& func) const
		{
			static_assert(FunctionTraits<Func>::NumArgs == 1, "func must take 1 argument");
			static_assert(std::is_same_v<ArgTypeN<Func, 0>, const Resource&>, "Function argument must be a 'const Resource&'");

			for (size_t i = 0; i < m_resources.size(); i++)
				if (!m_free_indices.contains(i))
					func(get_resource(i));
		}
		template <typename Func>
		void for_each(const Func&& func)
		{
			static_assert(FunctionTraits<Func>::NumArgs == 1, "func must take 1 argument");
			static_assert(std::is_same_v<ArgTypeN<Func, 0>, Resource&>, "Function argument must be a 'Resource&'");

			for (size_t i = 0; i < m_resources.size(); i++)
				if (!m_free_indices.contains(i))
					func(get_resource(i));
		}

		class ResourceIterator
		{
			ResourceManager& m_resource_manager;
			size_t m_index;

		public:
			ResourceIterator(ResourceManager& resource_manager, size_t index)
				: m_resource_manager(resource_manager), m_index(index)
			{
				while (m_index < m_resource_manager.m_resources.size() && m_resource_manager.m_free_indices.contains(m_index))
					++m_index;
			}
			ResourceIterator& operator++()
			{
				do { ++m_index; }
				while (m_index < m_resource_manager.m_resources.size() && m_resource_manager.m_free_indices.contains(m_index));
				return *this;
			}

			Resource& operator*() { return m_resource_manager.get_resource(m_index); }
			bool operator!=(const ResourceIterator& other) const { return m_index != other.m_index; }
		};

		class ConstResourceIterator
		{
			const ResourceManager& m_resource_manager;
			size_t m_index;

		public:
			ConstResourceIterator(const ResourceManager& resource_manager, size_t index)
				: m_resource_manager(resource_manager), m_index(index)
			{
				while (m_index < m_resource_manager.m_resources.size() && m_resource_manager.m_free_indices.contains(m_index))
					++m_index;
			}
			ConstResourceIterator& operator++()
			{
				do { ++m_index; }
				while (m_index < m_resource_manager.m_resources.size() && m_resource_manager.m_free_indices.contains(m_index));
				return *this;
			}

			const Resource& operator*() { return m_resource_manager.get_resource(m_index); }
			bool operator!=(const ConstResourceIterator& other) const { return m_index != other.m_index; }
		};

		ResourceIterator begin()            { return ResourceIterator(*this, 0); }
		ResourceIterator end()              { return ResourceIterator(*this, m_resources.size()); }
		ConstResourceIterator begin() const { return ConstResourceIterator(*this, 0); }
		ConstResourceIterator end()   const { return ConstResourceIterator(*this, m_resources.size()); }
		ConstResourceIterator cbegin() const noexcept { return begin(); }
		ConstResourceIterator cend()   const noexcept { return end(); }

	private:
		[[nodiscard]] Resource& get_resource(size_t p_index)
		{
			ASSERT_THROW(m_free_indices.contains(p_index) == false, "Trying to access a free p_index!");
			return m_resources[p_index]->m_resource;
		}
		[[nodiscard]] const Resource& get_resource(size_t p_index) const
		{
			ASSERT_THROW(m_free_indices.contains(p_index) == false, "Trying to access a free p_index!");
			return m_resources[p_index]->m_resource;
		}

		[[nodiscard]] size_t& get_counter(size_t p_index)
		{
			ASSERT_THROW(m_free_indices.contains(p_index) == false, "Trying to access a free p_index!");
			return m_resources[p_index]->m_count;
		}

		// Increment the count for resource at p_index.
		void increment(size_t p_index)
		{
			get_counter(p_index)++;
			if constexpr (LOG_REF_EVENTS) LOG("[ResourceManager] Incremented ResourceRef at index {} with count {}", p_index, get_counter(p_index));
		}
		// Decrement the count for ResourceData at p_index.
		// If the count reaches 0 then the ResourceData is removed from the manager.
		void decrement(size_t p_index)
		{
			ASSERT_THROW(m_free_indices.contains(p_index) == false, "Trying to access a free p_index!");
			if constexpr (LOG_REF_EVENTS) LOG("[ResourceManager] Decremented ResourceRef at index {} with count {}", p_index, get_counter(p_index) - 1);

			if (--get_counter(p_index) == 0)
				erase(p_index);
		}
		void erase(size_t index)
		{
			// Erase maintains the index order of m_resources making all the ResourceRefs remain valid after a 'resize'.
			if (index == m_resources.size() - 1)
			{// Erasing the last element in the buffer.
				m_resources.pop_back();
			}
			else
			{// Erasing an element in the middle of the buffer. Leaves gap that can be used constructing new Resource.
				m_resources[index].reset();
				m_free_indices.insert(index);
			}
			if constexpr (LOG_REF_EVENTS) LOG("[ResourceManager] Erased ResourceRef at index {}", index);
		}
	};
	// A ResourceRef is a non-owning pointer to a Resource managed by a ResourceManager.
	// When the last ResourceRef to a Resource is destroyed, the Resource is removed from the ResourceManager.
	template<typename Resource>
	class ResourceRef
	{
		using Manager = ResourceManager<Resource>;

		Manager* m_manager;            // A non-owning pointer to the ResourceManager that owns the resource.
		std::optional<size_t> m_index; // The index of the ResourceData in the ResourceManager, used as opposed to a pointer to avoid dangling pointers when m_resources is resized.

		// The ResourceManager is a friend so it can access the only valid constructor (private).
		friend Manager;
		ResourceRef(Manager& p_manager, size_t p_index) noexcept : m_manager(&p_manager), m_index(p_index)
		{
			if constexpr (LOG_REF_EVENTS) LOG("[ResourceRef] Constructed valid at address {} at index {}", (void*)(this), m_index.value());
			p_manager.increment(*m_index);
		}

	public:
		// Default construct an invalid ResourceRef. Equivalent to constructing a nullopt optional in std.
		ResourceRef() noexcept
			: m_manager{nullptr}
			, m_index(std::nullopt)
		{
			if constexpr (LOG_REF_EVENTS) LOG("[ResourceRef] Constructed empty at address {}", (void*)(this));
		}
		// On destory decrement the referece count for the manager.
		~ResourceRef() noexcept
		{
			if (has_value())
				m_manager->decrement(*m_index);

			if constexpr (LOG_REF_EVENTS) LOG("[ResourceRef] Destroyed at address {}", (void*)(this));
		}

		// On copy construct, copy the index and manager ptr and increment the count.
		ResourceRef(const ResourceRef& p_other) noexcept
			: m_manager{p_other.m_manager}
			, m_index{p_other.m_index}
		{
			if (has_value())
				m_manager->increment(*m_index);

			if constexpr (LOG_REF_EVENTS) LOG("[ResourceRef] Copy-constructing {} from {}", (void*)(this), (void*)(&p_other));
		}
		// On copy assigment, decrement the count for the current resource and assign this ResourceRef p_other data.
		ResourceRef& operator=(const ResourceRef& p_other) noexcept
		{
			// If the resource is the same one managed by other, we can safely skip the decrement and increments since the net ResourceRef change will be 0
			if (this != &p_other)
			{
				if (has_value())
					m_manager->decrement(*m_index);

				m_manager = p_other.m_manager;
				m_index   = p_other.m_index;

				if (has_value())
					m_manager->increment(*m_index);
			}

			if constexpr (LOG_REF_EVENTS) LOG("[ResourceRef] Copy-assigning {} from {}", (void*)(this), (void*)(&p_other));
			return *this;
		}
		// On move construct, move the resource ptr and manager ptr and index. Leave the old ResourceRef in an invalid state.
		ResourceRef(ResourceRef&& p_other) noexcept
			: m_manager{std::exchange(p_other.m_manager, nullptr)}
			, m_index{std::exchange(p_other.m_index, std::nullopt)}
		{
			if constexpr (LOG_REF_EVENTS) LOG("[ResourceRef] Move-constructing {} from {}", (void*)(this), (void*)(&p_other));
		}
		// On move assignment, decrement the count for the current resource and steal p_other's data.
		ResourceRef& operator=(ResourceRef&& p_other) noexcept
		{
			if (this != &p_other)
			{
				if (has_value())
					m_manager->decrement(*m_index);

				m_manager = std::exchange(p_other.m_manager, nullptr);
				m_index   = std::exchange(p_other.m_index, std::nullopt);
			}
			if constexpr (LOG_REF_EVENTS) LOG("[ResourceRef] Move-assigning {} from {}", (void*)(this), (void*)(&p_other));
			return *this;
		}

		constexpr const Resource* operator->() const noexcept   { return &m_manager->get_resource(m_index.value()); };
		constexpr Resource* operator->() noexcept               { return &m_manager->get_resource(m_index.value()); };
		constexpr const Resource& operator*() const& noexcept   { return m_manager->get_resource(m_index.value()); };
		constexpr Resource& operator*() & noexcept              { return m_manager->get_resource(m_index.value()); };
		constexpr const Resource&& operator*() const&& noexcept { return m_manager->get_resource(m_index.value()); };
		constexpr Resource&& operator*() && noexcept            { return m_manager->get_resource(m_index.value()); };
		constexpr Resource& value() noexcept                    { return m_manager->get_resource(m_index.value()); };
		constexpr const Resource& value() const noexcept        { return m_manager->get_resource(m_index.value()); };
		constexpr bool has_value() const noexcept               { return m_manager != nullptr; };
		constexpr explicit operator bool() const noexcept       { return has_value(); };
		constexpr operator Resource&() noexcept                 { return m_manager->get_resource(m_index.value()); }
		constexpr operator const Resource&() const noexcept     { return m_manager->get_resource(m_index.value()); }
	};
} // namespace Utility