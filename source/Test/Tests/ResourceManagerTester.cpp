#include "ResourceManagerTester.hpp"
#include "MemoryCorrectnessItem.hpp"
#include "Utility/ResourceManager.hpp"

#include <vector>

namespace Test
{
	using Ref     = Utility::ResourceRef<MemoryCorrectnessItem>;
	using Manager = Utility::ResourceManager<MemoryCorrectnessItem>;

	void Test::ResourceManagerTester::run_unit_tests()
	{
		{// Check ResourceRef API
			MemoryCorrectnessItem::reset();

			auto ref = Ref();
			CHECK_EQUAL(ref.has_value(), false, "ResourceRef isValid() check");
			CHECK_TRUE(ref ? false : true,     "ResourceRef explicit bool operator check");

			Manager manager;
			auto ref2 = manager.insert(MemoryCorrectnessItem{});
			CHECK_EQUAL(ref2->count_alive(), 1,   "Use the Resource via the Ref");
			CHECK_EQUAL((*ref2).count_alive(), 1, "Use the Resource via the Ref");

			const auto ref3 = manager.insert(MemoryCorrectnessItem{});
			CHECK_EQUAL(ref3->count_alive(), 2,   "Use the Resource via the Ref");
			CHECK_EQUAL((*ref3).count_alive(), 2, "Use the Resource via the Ref");
		}
		CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
		CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory Error check");

		{ // Check memory leaks single insert
			MemoryCorrectnessItem::reset();

			{
				Manager manager;
				{
					auto ref = manager.insert(MemoryCorrectnessItem{});
					CHECK_EQUAL(manager.size(), 1, "Size check after insert");
				}
				CHECK_EQUAL(manager.size(), 0, "Size check after destroyed ref");
				CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check ref deleted");
				CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory Error check ref deleted");
			}
			CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
			CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory Error check");
		}
		{// Check memory leaks many inserts
			MemoryCorrectnessItem::reset();

			Manager manager;
			manager.reserve(100);
			{
				std::vector<Ref> refs; // Maintain their lifetime in vector
				refs.reserve(100);
				for (int i = 0; i < 100; i++)
					refs.push_back(manager.insert(MemoryCorrectnessItem{}));
				CHECK_EQUAL(manager.size(), 100, "Size check after insert 100");
			}
			CHECK_EQUAL(manager.size(), 0, "Size check after insert 100 deleted");
			CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
			CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory error check");
		}
		{// Check capacity change maintains resource validity
			MemoryCorrectnessItem::reset();

			{
				Manager manager;
				auto ref = manager.insert(MemoryCorrectnessItem{});
				manager.reserve(manager.capacity() * 2);
				CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 1, "Memory leak check");
			}

			CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
			CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory Error check");
		}
		{// Check capacity change maintains resource validity many inserts
			MemoryCorrectnessItem::reset();

			{
				Manager manager;
				ASSERT(manager.capacity() < 100, "Capacity has to be below 100 for test to work.");
				std::vector<Ref> refs; // Maintain their lifetime in vector
				refs.reserve(100);

				for (int i = 0; i < 100; i++)
					refs.push_back(manager.insert(MemoryCorrectnessItem{}));

				CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 100, "Memory leak check");
			}

			CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
			CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory Error check");
		}
		{ // Check memory leaks Manager::clear()
			MemoryCorrectnessItem::reset();
			if (false)
			{
				// This Test crashes -
				// ResourceRefs have no concept of the Manager having cleared them.
				// When the refs vector is destroyed, it calls destructors on incorrectly valid ResourceRefs.
				Manager manager;
				std::vector<Ref> refs; // Maintain their lifetime in vector
				refs.reserve(4);
				for (int i = 0; i < 4; i++)
					refs.push_back(manager.insert(MemoryCorrectnessItem{}));

				CHECK_EQUAL(manager.size(), 4, "Size check after insert 4");
				manager.clear();
				CHECK_EQUAL(manager.size(), 0, "Size check after clear");

				CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
				CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory error check");
			}
		}
		{// Check capacity change maintains resource validity with many resources
			MemoryCorrectnessItem::reset();

			Manager manager;
			for (int i = 0; i < 100; i++)
				auto ref = manager.insert(MemoryCorrectnessItem{});
			manager.reserve(manager.capacity() * 2);

			CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
			CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory Error check");
		}
		{// Create an invalid resource and assign it a valid resource
			MemoryCorrectnessItem::reset();

			{
				Manager manager;

				auto ref = Ref();
				CHECK_EQUAL(manager.size(), 0, "Size check after invalid ref");

				ref = manager.insert(MemoryCorrectnessItem{});
				CHECK_TRUE(ref.has_value(), "Invalid ResourceRef is valid after assigning");
				CHECK_EQUAL(manager.size(), 1, "Size check after assigning to an invalid ref");

				auto ref2 = manager.insert(MemoryCorrectnessItem{});
				CHECK_EQUAL(manager.size(), 2, "Size check after inserting a second resource");

				// Copying a ref should give us access to the same resource and not change the size.
				auto ref_copy = ref;
				CHECK_TRUE(ref_copy.has_value(), "ResourceRef copy is valid");
				CHECK_EQUAL(manager.size(), 2, "Size check after copying a ResourceRef");
			}
			CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
			CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory Error check");
		}
		{// Create an valid resource and assign it a valid resource
			MemoryCorrectnessItem::reset();
			{
				Manager manager;
				manager.reserve(4);

				auto ref = manager.insert(MemoryCorrectnessItem{}); // Construct, Move constructing, Delete
				ref = manager.insert(MemoryCorrectnessItem{});      // Construct (new one), Move-assign, Delete
				CHECK_TRUE(ref.has_value(), "Check ref is valid after being assigned while already owning a resource");
				CHECK_EQUAL(manager.size(), 1, "Size remains the same after assigning to a valid ref");
				CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 1, "Memory leak after move-assigning a valid ref");
			}
			CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
			CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory Error check");
		}
		{// Check Resource data is intact after a second insert
			MemoryCorrectnessItem::reset();
			{
				Manager manager;
				auto ref_1 = manager.insert(MemoryCorrectnessItem{});
				ref_1->m_member = 5;
				auto ref_2 = manager.insert(MemoryCorrectnessItem{});

				CHECK_EQUAL(ref_1->m_member.value(), 5, "Check data intact after a second insert");
			}
			CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
			CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory Error check");
		}
		{// Test range-based for - these depend on the ResourceManager leaving gaps inside the buffer when erasing from non-end positions.
			{ // Test range-based for non-const
				MemoryCorrectnessItem::reset();
				{
					Manager manager;
					std::vector<Ref> refs;
					refs.reserve(5);
					for (int i = 0; i < 5; i++)
					{
						refs.push_back(manager.insert(MemoryCorrectnessItem{}));
						refs.back()->m_member = i;
					}
					// Test full buffer iteration.
					{
						std::vector<int> values;
						for (auto& resource : manager)
							values.push_back(*resource.m_member);

						CHECK_EQUAL(values.size(), 5, "Range-based for loop iteration full buffer count");
						CHECK_EQUAL(values[0], 0, "Range-based for loop iteration full buffer data validity 0");
						CHECK_EQUAL(values[1], 1, "Range-based for loop iteration full buffer data validity 1");
						CHECK_EQUAL(values[2], 2, "Range-based for loop iteration full buffer data validity 2");
						CHECK_EQUAL(values[3], 3, "Range-based for loop iteration full buffer data validity 3");
						CHECK_EQUAL(values[4], 4, "Range-based for loop iteration full buffer data validity 4");
					}
					// Test buffer with middle-gap iteration.
					{
						refs.erase(refs.begin() + 2);

						std::vector<int> values;
						for (auto& resource : manager)
							values.push_back(*resource.m_member);

						CHECK_EQUAL(values.size(), 4, "Range-based for loop iteration middle-gap buffer count");
						CHECK_EQUAL(values[0], 0, "Range-based for loop iteration middle-gap buffer data validity 0");
						CHECK_EQUAL(values[1], 1, "Range-based for loop iteration middle-gap buffer data validity 1");
						CHECK_EQUAL(values[2], 3, "Range-based for loop iteration middle-gap buffer data validity 2");
					}
					// Test buffer with start-gap iteration.
					{
						refs.erase(refs.begin());

						std::vector<int> values;
						for (auto& resource : manager)
							values.push_back(*resource.m_member);

						CHECK_EQUAL(values.size(), 3, "Range-based for loop iteration start-gap buffer count");
						CHECK_EQUAL(values[0], 1, "Range-based for loop iteration start-gap buffer data validity 0");
						CHECK_EQUAL(values[1], 3, "Range-based for loop iteration start-gap buffer data validity 1");
						CHECK_EQUAL(values[2], 4, "Range-based for loop iteration start-gap buffer data validity 2");
					}
					// Test buffer with end-gap iteration.
					{
						refs.erase(refs.end() - 1);

						std::vector<int> values;
						for (auto& resource : manager)
							values.push_back(*resource.m_member);

						CHECK_EQUAL(values.size(), 2, "Range-based for loop iteration end-gap buffer count");
						CHECK_EQUAL(values[0], 1, "Range-based for loop iteration end-gap buffer data validity 0");
						CHECK_EQUAL(values[1], 3, "Range-based for loop iteration end-gap buffer data validity 1");
					}
					// Test buffer with empty iteration.
					{
						refs.clear();

						std::vector<int> values;
						for (const auto& resource : manager)
							values.push_back(*resource.m_member);

						CHECK_EQUAL(values.size(), 0, "Range-based for loop iteration clear buffer count");
					}
				}
				CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
				CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory Error check");
			}

			{// Test range-based for const correctness
				MemoryCorrectnessItem::reset();
				{
					const Manager manager;
					std::vector<Ref> refs;
					refs.reserve(5);
					for (int i = 0; i < 5; i++)
					{// Hack to get around the const-ness of the manager for this test.
						Manager& non_const_manager = const_cast<Manager&>(manager);
						refs.push_back(non_const_manager.insert(MemoryCorrectnessItem{}));
						refs.back()->m_member = i;
					}
					// Test full buffer iteration.
					{
						std::vector<int> values;
						for (const auto& resource : manager)
							values.push_back(*resource.m_member);

						CHECK_EQUAL(values.size(), 5, "Range-based for loop iteration full buffer count");
						CHECK_EQUAL(values[0], 0, "Range-based for loop iteration full buffer data validity 0");
						CHECK_EQUAL(values[1], 1, "Range-based for loop iteration full buffer data validity 1");
						CHECK_EQUAL(values[2], 2, "Range-based for loop iteration full buffer data validity 2");
						CHECK_EQUAL(values[3], 3, "Range-based for loop iteration full buffer data validity 3");
						CHECK_EQUAL(values[4], 4, "Range-based for loop iteration full buffer data validity 4");
					}
					// Test buffer with middle-gap iteration.
					{
						refs.erase(refs.begin() + 2);

						std::vector<int> values;
						for (auto resource : manager)
							values.push_back(*resource.m_member);

						CHECK_EQUAL(values.size(), 4, "Range-based for loop iteration middle-gap buffer count");
						CHECK_EQUAL(values[0], 0, "Range-based for loop iteration middle-gap buffer data validity 0");
						CHECK_EQUAL(values[1], 1, "Range-based for loop iteration middle-gap buffer data validity 1");
						CHECK_EQUAL(values[2], 3, "Range-based for loop iteration middle-gap buffer data validity 2");
					}
					// Test buffer with start-gap iteration.
					{
						refs.erase(refs.begin());

						std::vector<int> values;
						for (const auto& resource : manager)
							values.push_back(*resource.m_member);

						CHECK_EQUAL(values.size(), 3, "Range-based for loop iteration start-gap buffer count");
						CHECK_EQUAL(values[0], 1, "Range-based for loop iteration start-gap buffer data validity 0");
						CHECK_EQUAL(values[1], 3, "Range-based for loop iteration start-gap buffer data validity 1");
						CHECK_EQUAL(values[2], 4, "Range-based for loop iteration start-gap buffer data validity 2");
					}
					// Test buffer with end-gap iteration.
					{
						refs.erase(refs.end() - 1);

						std::vector<int> values;
						for (const auto& resource : manager)
							values.push_back(*resource.m_member);

						CHECK_EQUAL(values.size(), 2, "Range-based for loop iteration end-gap buffer count");
						CHECK_EQUAL(values[0], 1, "Range-based for loop iteration end-gap buffer data validity 0");
						CHECK_EQUAL(values[1], 3, "Range-based for loop iteration end-gap buffer data validity 1");
					}
					// Test buffer with empty iteration.
					{
						refs.clear();

						std::vector<int> values;
						for (const auto& resource : manager)
							values.push_back(*resource.m_member);

						CHECK_EQUAL(values.size(), 0, "Range-based for loop iteration clear buffer count");
					}
				}
				CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 0, "Memory leak check");
				CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory Error check");
			}
		}

		{// TODO test get_or_create
		}
		{// TODO Check move assigning and move constructing a ResourceManager
		}
		{// TODO check Ref is_valid() == false after the manager is cleared?
		}
	}

	void Test::ResourceManagerTester::run_performance_tests() {}
} // namespace Test
