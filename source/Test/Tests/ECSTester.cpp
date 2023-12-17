#include "ECSTester.hpp"
#include "MemoryCorrectnessItem.hpp"

#include "ECS/Storage.hpp"
#include "Utility/Logger.hpp"

#include <set>
#include <algorithm>
#include <vector>
#include <random>
#include <chrono>

DISABLE_WARNING_PUSH
DISABLE_WARNING_UNUSED_VARIABLE // Required to stop variables being destroyed before they are used in tests.

namespace Test
{
	// Tests if any MemoryCorrectnessErrors occurred and if the number of items alive matches p_alive_count_expected
	void ECSTester::run_memory_test(const size_t& p_alive_count_expected)
	{
		CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Check memory errors");
		CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), p_alive_count_expected, "Check alive count");
	}

	void ECSTester::run_performance_tests()
	{}

	void ECSTester::run_unit_tests()
	{SCOPE_SECTION("ECS");
		{SCOPE_SECTION("count_entities")

			ECS::Storage storage;
			CHECK_EQUAL(storage.count_entities(), 0, "Start Empty");
			std::optional<ECS::Entity> double_ent;
			std::optional<ECS::Entity> float_ent;
			std::optional<ECS::Entity> float_and_double_ent;

			{SCOPE_SECTION("Add entity")

				double_ent = storage.add_entity(42.0);
				CHECK_EQUAL(storage.count_entities(), 1, "Add single component entity");

				float_ent = storage.add_entity(13.f);
				CHECK_EQUAL(storage.count_entities(), 2, "Add new archetype entity");

				float_and_double_ent = storage.add_entity(13.f, 42.0);
				CHECK_EQUAL(storage.count_entities(), 3, "Add another entity with both component types");
			}
			{SCOPE_SECTION("Delete entity");

				storage.delete_entity(double_ent.value());
				CHECK_EQUAL(storage.count_entities(), 2, "Delete entity");

				storage.delete_entity(float_ent.value());
				CHECK_EQUAL(storage.count_entities(), 1, "Delete another entity");

				storage.delete_entity(float_and_double_ent.value());
				CHECK_EQUAL(storage.count_entities(), 0, "Delete last entity");
			}
		}

		{SCOPE_SECTION("count_components")

			ECS::Storage storage;
			CHECK_EQUAL(storage.count_components<double>(), 0, "Start Empty");
			std::optional<ECS::Entity> double_ent;
			std::optional<ECS::Entity> float_ent;
			std::optional<ECS::Entity> float_and_double_ent;

			{SCOPE_SECTION("Add component");

				double_ent = storage.add_entity(42.0);
				CHECK_EQUAL(storage.count_components<double>(), 1, "Add double ent");
				CHECK_EQUAL(storage.count_components<float>(),  0, "Add double ent check float");
				CHECK_EQUAL(storage.count_components<int>(),    0, "Add double ent check int");

				float_ent = storage.add_entity(13.f);
				CHECK_EQUAL(storage.count_components<double>(), 1, "Add float ent check double");
				CHECK_EQUAL(storage.count_components<float>(),  1, "Add float ent check float");
				CHECK_EQUAL(storage.count_components<int>(),    0, "Add float ent check int");

				float_and_double_ent = storage.add_entity(13.f, 42.0);
				CHECK_EQUAL(storage.count_components<double>(), 2, "Add float and double ent check double");
				CHECK_EQUAL(storage.count_components<float>(),  2, "Add float and double ent check float");
				CHECK_EQUAL(storage.count_components<int>(),    0, "Count type not in storage");
				auto count_combo = storage.count_components<double, float>(); // comma in template args is not supported by CHECK_EQUAL
				CHECK_EQUAL(count_combo, 1, "Add float and double ent check combo");
			}

			{SCOPE_SECTION("Delete entity")
				storage.delete_entity(double_ent.value());
				CHECK_EQUAL(storage.count_components<double>(), 1, "Remove double ent check double");
				CHECK_EQUAL(storage.count_components<float>(),  2, "Remove double ent check float");

				storage.delete_component<float>(float_and_double_ent.value());
				CHECK_EQUAL(storage.count_components<double>(), 1, "Remove float from float_and_double ent check double");
				CHECK_EQUAL(storage.count_components<float>(),  1, "Remove float from float_and_double ent check float");
				auto count_combo = storage.count_components<double, float>(); // comma in template args is not supported by CHECK_EQUAL
				CHECK_EQUAL(count_combo, 0, "Remove float from float_and_double ent check combo");

				storage.delete_entity(float_and_double_ent.value());
				CHECK_EQUAL(storage.count_components<double>(), 0, "Remove float and double ent check double");
				CHECK_EQUAL(storage.count_components<float>(),  1, "Remove float and double ent check float");

				storage.delete_entity(float_ent.value());
				CHECK_EQUAL(storage.count_components<double>(), 0, "Remove float ent check double");
				CHECK_EQUAL(storage.count_components<float>(),  0, "Remove float ent check float");
			}
		}

		{SCOPE_SECTION("add_entity");
			{
				ECS::Storage storage;
				const float float_comp   = 42.f;
				const double double_comp = 13.0;

				run_memory_test(0);

				storage.add_entity(float_comp);
				CHECK_EQUAL(storage.count_entities(), 1, "Add single component entity");

				storage.add_entity(double_comp);
				CHECK_EQUAL(storage.count_entities(), 2, "Add another single component entity");

				storage.add_entity(double_comp, float_comp);
				CHECK_EQUAL(storage.count_entities(), 3, "Add another entity with both component types");
			}
			{SCOPE_SECTION("Memory correctness");
				MemoryCorrectnessItem::reset(); // Reset before starting new tests
				ECS::Storage storage;
				MemoryCorrectnessItem comp;

				{SCOPE_SECTION("Add by copy");
					storage.add_entity(comp);
					run_memory_test(2);
				}
				{SCOPE_SECTION("Add second copy");
					storage.add_entity(comp);
					run_memory_test(3);
				}
				{SCOPE_SECTION("New archetype");
					storage.add_entity(1.f);
					run_memory_test(3); // Should still be 3 alive because we didnt add another mem correctness item
				}
				{SCOPE_SECTION("Add by move");
					storage.add_entity(MemoryCorrectnessItem());
					run_memory_test(4); // Should now be 4 alive because we move constructed a new one into storage
				}
				{SCOPE_SECTION("Add 100");
					for (int i = 0; i < 100; i++)
						storage.add_entity(MemoryCorrectnessItem());

					run_memory_test(104);
				}
			}
		}

		{SCOPE_SECTION("delete_entity");
			{
				ECS::Storage storage;

				auto ent = storage.add_entity(1.f);
				CHECK_EQUAL(storage.count_entities(), 1, "Add 1 entity");

				storage.delete_entity(ent);
				CHECK_EQUAL(storage.count_entities(), 0, "Add 1 entity then delete");
			}
			{SCOPE_SECTION("Memory correctness");
				{
					MemoryCorrectnessItem::reset(); // Reset before starting new tests
					ECS::Storage storage;

					auto ent = storage.add_entity(MemoryCorrectnessItem());
					storage.delete_entity(ent);
					run_memory_test(0);
				}

				{SCOPE_SECTION("Destroy storage with entity still alive");
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;
						storage.add_entity(MemoryCorrectnessItem());
					}
					run_memory_test(0); // Dangling memory check
				}

				{SCOPE_SECTION("Add 3 delete back to front"); // Back to front is easiest to deal with for removing, no moving is required.
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;
						auto front_ent  = storage.add_entity(MemoryCorrectnessItem());
						auto middle_ent = storage.add_entity(MemoryCorrectnessItem());
						auto back_ent   = storage.add_entity(MemoryCorrectnessItem());

						storage.delete_entity(back_ent);
						CHECK_EQUAL(storage.count_entities(), 2, "First delete");
						run_memory_test(2);

						storage.delete_entity(middle_ent);
						CHECK_EQUAL(storage.count_entities(), 1, "Second delete");
						run_memory_test(1);

						storage.delete_entity(front_ent);
						CHECK_EQUAL(storage.count_entities(), 0, "Third delete");
						run_memory_test(0);
					}
					run_memory_test(0); // Dangling memory check
				}

				{SCOPE_SECTION("Add 3 delete front to back"); // Front to back is the worst case removal requiring moving of all items.
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;
						auto front_ent  = storage.add_entity(MemoryCorrectnessItem());
						auto middle_ent = storage.add_entity(MemoryCorrectnessItem());
						auto back_ent   = storage.add_entity(MemoryCorrectnessItem());

						storage.delete_entity(front_ent);
						CHECK_EQUAL(storage.count_entities(), 2, "First delete");
						run_memory_test(2);

						storage.delete_entity(middle_ent);
						CHECK_EQUAL(storage.count_entities(), 1, "Second delete");
						run_memory_test(1);

						storage.delete_entity(back_ent);
						CHECK_EQUAL(storage.count_entities(), 0, "Third delete");
						run_memory_test(0);
					}
					run_memory_test(0); // Dangling memory check
				}
				{SCOPE_SECTION("Add 3 delete middle -> front -> back");
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;
						auto front_ent  = storage.add_entity(MemoryCorrectnessItem());
						auto middle_ent = storage.add_entity(MemoryCorrectnessItem());
						auto back_ent   = storage.add_entity(MemoryCorrectnessItem());

						storage.delete_entity(middle_ent);
						CHECK_EQUAL(storage.count_entities(), 2, "First delete");
						run_memory_test(2);

						storage.delete_entity(front_ent);
						CHECK_EQUAL(storage.count_entities(), 1, "Second delete");
						run_memory_test(1);

						storage.delete_entity(back_ent);
						CHECK_EQUAL(storage.count_entities(), 0, "Third delete");
						run_memory_test(0);
					}
					run_memory_test(0); // Dangling memory check
				}
				{SCOPE_SECTION("Add 100 delete 100 in random order");
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;

						std::vector<ECS::Entity> entities;
						entities.reserve(100);

						for (size_t i = 0; i < 100; i++)
							entities.push_back(storage.add_entity(MemoryCorrectnessItem()));

						// shuffle the order of entities
						auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
						auto e = std::default_random_engine(seed);
						std::shuffle(entities.begin(), entities.end(), e);

						for (auto& ent : entities)
							storage.delete_entity(ent);

						run_memory_test(0);
					}
					run_memory_test(0); // Dangling memory check
				}
				{SCOPE_SECTION("Overwrite memory");
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;

						auto ent = storage.add_entity(MemoryCorrectnessItem());
						storage.delete_entity(ent);
						storage.add_entity(MemoryCorrectnessItem());
						CHECK_EQUAL(storage.count_entities(), 1, "Overwrite Add -> Delete -> Add");
						run_memory_test(1);
					}
					run_memory_test(0);
				}
				{SCOPE_SECTION("Overwrite memory test 100");
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;

						std::vector<ECS::Entity> entities;
						entities.reserve(100);

						for (int i = 0; i < 100; i++)
							entities.push_back(storage.add_entity(MemoryCorrectnessItem()));

						for (auto& ent : entities)
							storage.delete_entity(ent);

						for (int i = 0; i < 100; i++)
							entities.push_back(storage.add_entity(MemoryCorrectnessItem()));

						CHECK_EQUAL(storage.count_entities(), 100, "Add 100, Delete 100, Add 100");
						run_memory_test(100);
					}
					run_memory_test(0);
				}
			}
		}

		{SCOPE_SECTION("get_component");

			{SCOPE_SECTION("const")
				ECS::Storage storage;
				auto entity = storage.add_entity(42.0);
				CHECK_EQUAL(storage.get_component<double>(entity), 42.0, "single component entity");

				// Signature variations
				CHECK_EQUAL(storage.get_component<double&>(entity), 42.0, "non-const & get");
				CHECK_EQUAL(storage.get_component<const double&>(entity), 42.0, "const & get");

				// std::decay doesnt work on * so the below dont compile for now.
				// CHECK_EQUAL(storage.get_component<double*>(entity), 42.0, "single component* entity");
				// CHECK_EQUAL(storage.get_component<const double*>(entity), 42.0, "single 'const component*' entity");

				{SCOPE_SECTION("double float bool entity");

					auto entity = storage.add_entity(1.0, 2.f, true);
					CHECK_EQUAL(storage.get_component<double>(entity), 1.0, "get double");
					CHECK_EQUAL(storage.get_component<float>(entity), 2.0f, "get float");
					CHECK_EQUAL(storage.get_component<bool>(entity),  true, "get bool");
				}
				{SCOPE_SECTION("bool float double entity"); // Reverse order but same components/archetype as previous

					auto entity_reverse = storage.add_entity(false, 1.f, 2.0);
					CHECK_EQUAL(storage.get_component<double>(entity_reverse), 2.0, "get double");
					CHECK_EQUAL(storage.get_component<float>(entity_reverse), 1.0f, "get float");
					CHECK_EQUAL(storage.get_component<bool>(entity_reverse), false, "get bool");
				}
				{SCOPE_SECTION("float bool double entity"); // Different order but same components/archetype as previous 2
					auto entity_new = storage.add_entity(13.f, true, 42.0);
					CHECK_EQUAL(storage.get_component<double>(entity_new), 42.0, "get double");
					CHECK_EQUAL(storage.get_component<float>(entity_new), 13.0f, "get float");
					CHECK_EQUAL(storage.get_component<bool>(entity_new),   true, "get bool");
				}
				{SCOPE_SECTION("char entity");
					auto entity_new = storage.add_entity('G');
					CHECK_EQUAL(storage.get_component<char>(entity_new), 'G', "get char");
				}

				{SCOPE_SECTION("Data limits") // Setting as many bits as possible
					constexpr double max_double = std::numeric_limits<double>::max();
					constexpr double min_double = std::numeric_limits<double>::min();

					auto entity_max_double_1 = storage.add_entity(max_double);
					auto entity_min_double_1 = storage.add_entity(min_double);
					auto entity_max_double_2 = storage.add_entity(max_double);
					auto entity_max_double_3 = storage.add_entity(max_double);
					auto entity_min_double_2 = storage.add_entity(min_double);
					auto entity_min_double_3 = storage.add_entity(min_double);
					auto entity_min_double_4 = storage.add_entity(min_double);
					auto entity_max_double_4 = storage.add_entity(max_double);

					CHECK_EQUAL(storage.get_component<double>(entity_max_double_1), max_double, "1");
					CHECK_EQUAL(storage.get_component<double>(entity_max_double_2), max_double, "2");
					CHECK_EQUAL(storage.get_component<double>(entity_max_double_3), max_double, "3");
					CHECK_EQUAL(storage.get_component<double>(entity_max_double_4), max_double, "4");
					CHECK_EQUAL(storage.get_component<double>(entity_min_double_1), min_double, "1");
					CHECK_EQUAL(storage.get_component<double>(entity_min_double_2), min_double, "2");
					CHECK_EQUAL(storage.get_component<double>(entity_min_double_3), min_double, "3");
					CHECK_EQUAL(storage.get_component<double>(entity_min_double_4), min_double, "4");
				}
			}
			{SCOPE_SECTION("non-const")
				ECS::Storage storage;

				{SCOPE_SECTION("Get and assign");
					auto entity  = storage.add_entity(42.0);
					auto& comp   = storage.get_component<double>(entity);
					comp         = 69.0;

					CHECK_EQUAL(storage.get_component<double>(entity), 69.0, "Value change after assign");

					storage.get_component<double>(entity) += 10.0;
					CHECK_EQUAL(storage.get_component<double>(entity), 79.0, "get and set one liner");

				}
				{SCOPE_SECTION("Get and assign second"); // Add to the same archetype
					auto entity = storage.add_entity(27.0);
					storage.get_component<double>(entity) += 3.0;
					CHECK_EQUAL(storage.get_component<double>(entity), 30.0, "get and set to same archetype");
				}

				{SCOPE_SECTION("Add new archetype ent");
					auto entity = storage.add_entity(27.0, 49.f);
					storage.get_component<double>(entity) += 3.0;
					CHECK_EQUAL(storage.get_component<double>(entity), 30.0, "check");

					storage.get_component<float>(entity) += 1.0f;
					CHECK_EQUAL(storage.get_component<float>(entity), 50.0f, "check 2");
				}
				{SCOPE_SECTION("double float int entity");
					auto entity = storage.add_entity(1.0, 2.f, 3);
					storage.get_component<int>(entity) += 1;
					CHECK_EQUAL(storage.get_component<int>(entity), 4, "Edit int");

					storage.get_component<float>(entity) += 19.0f;
					CHECK_EQUAL(storage.get_component<float>(entity), 21.f, "Edit float");

					storage.get_component<double>(entity) += 13.0;
					CHECK_EQUAL(storage.get_component<double>(entity), 14.0, "Edit double");
				}
			}
			{SCOPE_SECTION("Memory correctness");
				{
					MemoryCorrectnessItem::reset();
					ECS::Storage storage;
					auto mem_correct_entity = storage.add_entity(MemoryCorrectnessItem());

					{SCOPE_SECTION("const");
						{SCOPE_SECTION("Return a reference");
							const auto& compRef = storage.get_component<MemoryCorrectnessItem>(mem_correct_entity);
							run_memory_test(1);
						}
						{SCOPE_SECTION("Return by copy");
							const auto compCopy = storage.get_component<MemoryCorrectnessItem>(mem_correct_entity);
							run_memory_test(2);
						}
					}
					{SCOPE_SECTION("non-const");
						{SCOPE_SECTION("Return a reference");
							auto& compRef = storage.get_component<MemoryCorrectnessItem>(mem_correct_entity);
							run_memory_test(1);
						}
						{SCOPE_SECTION("Return by copy");
							const auto compCopy = storage.get_component<MemoryCorrectnessItem>(mem_correct_entity);
							run_memory_test(2);
						}
					}
				}

				run_memory_test(0);
			}
		}

		{SCOPE_SECTION("has_components")

			ECS::Storage storage;
			const auto double_float_bool_ent = storage.add_entity(1.0, 2.f, true);
			const auto double_ent            = storage.add_entity(1.0);

			{
				auto has_components = storage.has_components<double>(double_ent);
				CHECK_EQUAL(has_components, true, "exact match single type single component");
			}
			{
				bool has_components = storage.has_components<double, float, bool>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "exact match multiple types");
			}
			{
				auto has_components = storage.has_components<bool, float, double>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "exact match different order multiple types");
			}
			{
				auto has_components = storage.has_components<float>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "single type match from multiple component middle");
			}
			{
				auto has_components = storage.has_components<double, bool>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "subset match");
			}
			{
				auto has_components = storage.has_components<bool, double>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "subset match different order");
			}
			{
				auto has_components = storage.has_components<double>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "subset match single type");
			}
			{
				auto has_components = storage.has_components<std::string>(double_float_bool_ent);
				CHECK_EQUAL(has_components, false, "no match single type");
			}
			{
				auto has_components = storage.has_components<std::string, size_t>(double_float_bool_ent);
				CHECK_EQUAL(has_components, false, "no match multiple types");
			}
		}

		{SCOPE_SECTION("foreach");
			{
				ECS::Storage storage;

				{SCOPE_SECTION("Iterate empty");

					size_t count      = 0;
					double sum_double = 0.0;
					float sum_float   = 0.0f;
					int sum_int       = 0;

					storage.foreach([&](double& p_double, float& p_float, int& p_int)
					{
						sum_double += p_double;
						sum_float  += p_float;
						sum_int    += p_int;
						count++;
					});

					CHECK_EQUAL(sum_double,  0.0, "Sum of doubles");
					CHECK_EQUAL(sum_float,  0.0f, "Sum of floats");
					CHECK_EQUAL(sum_int,       0, "Sum of ints");
					CHECK_EQUAL(count, 0, "Iterate count");
				}

				storage.add_entity(13.69, 1.33f, 2);
				storage.add_entity(13.69, 1.33f, 2);
				storage.add_entity(13.69, 1.33f, 2);

				{SCOPE_SECTION("Exact match and order to archetype");
					size_t count = 0;
					storage.foreach([&](double& p_double, float& p_float, int& p_int)
					{
						CHECK_EQUAL(p_double, 13.69, "Check double");
						CHECK_EQUAL(p_int,        2, "Check int");
						CHECK_EQUAL(p_float,  1.33f, "Check float");
						count++;
					});
					CHECK_EQUAL(count, 3, "Iterate count");
				}
				{SCOPE_SECTION("Exact match different order to archetype");
					size_t count = 0;
					storage.foreach([&](float& p_float, int& p_int, double& p_double)
					{
						CHECK_EQUAL(p_double, 13.69, "Check double");
						CHECK_EQUAL(p_int,        2, "Check int");
						CHECK_EQUAL(p_float,  1.33f, "Check float");
						count++;
					});
					CHECK_EQUAL(count, 3, "Ieration count");
				}
				{SCOPE_SECTION("Subset match same order to archetype");
					size_t count = 0;
					storage.foreach([&](double& p_double, float& p_float)
					{
						CHECK_EQUAL(p_double, 13.69, "Check double");
						CHECK_EQUAL(p_float,  1.33f, "Check float");
						count++;
					});
					CHECK_EQUAL(count, 3, "Ieration count");
				}
				{SCOPE_SECTION("Subset match different order to archetype");
					size_t count = 0;
					storage.foreach([&](int& p_int, float& p_float)
					{
						CHECK_EQUAL(p_int,       2, "Check int");
						CHECK_EQUAL(p_float, 1.33f, "Check float");
						count++;
					});
					CHECK_EQUAL(count, 3, "Ieration count");
				}
				{SCOPE_SECTION("Single argument match to archetype");
					{SCOPE_SECTION("Front");
						size_t count = 0;
						storage.foreach([&](double& p_double)
						{
							CHECK_EQUAL(p_double, 13.69, "Check double");
							count++;
						});
						CHECK_EQUAL(count, 3, "Ieration count");
					}
					{SCOPE_SECTION("Middle");
						size_t count = 0;
						storage.foreach([&](float& p_float)
						{
							CHECK_EQUAL(p_float, 1.33f, "Check float");
							count++;
						});
						CHECK_EQUAL(count, 3, "Ieration count");
					}
					{SCOPE_SECTION("Back");
						size_t count = 0;
						storage.foreach([&](int& p_int)
						{
							CHECK_EQUAL(p_int, 2, "Check int");
							count++;
						});
						CHECK_EQUAL(count, 3, "Ieration count");
					}
				}
				{SCOPE_SECTION("Exact match change data");
					size_t count = 0;
					storage.foreach([&](double& p_double, float& p_float, int& p_int)
					{
						p_double += 1.0;
						p_float  += 1.0f;
						p_int    += 1;
						count++;
					});
					CHECK_EQUAL(count, 3, "Ieration count");
				}
				{SCOPE_SECTION("Exact match check changed data");
					storage.foreach([&](double& p_double, float& p_float, int& p_int)
					{
						CHECK_EQUAL(p_double, 14.69, "Check double");
						CHECK_EQUAL(p_int,        3, "Check int");
						CHECK_EQUAL(p_float,  2.33f, "Check float");
					});
				}
				{SCOPE_SECTION("Add a new entity to a new archetype");
					storage.add_entity(13.0);
					size_t count = 0;
					double sum = 0.0;
					storage.foreach([&](double& p_double)
					{
						sum += p_double;
						count++;
					});

					CHECK_EQUAL(sum, 57.07, "Sum of doubles"); // 14.69 * 3 + 13.0 = 57.07
					CHECK_EQUAL(count, 4, "Iteration count");
				}
			}

			{SCOPE_SECTION("Entity argument") // ECS::Entity inside the foreach func arguments, expecting the Entity passed with its owned components

				ECS::Storage storage;

				std::vector<ECS::Entity> entities;
				for (int i = 0; i < 12; i++)
					entities.push_back(storage.add_entity(1.0, 2.f, true));

				{SCOPE_SECTION("Iterate Entity only")

					std::set<ECS::Entity> entity_set;
					storage.foreach([&](ECS::Entity& p_entity){ entity_set.insert(p_entity); });

					for (const auto& entity : entities)
						CHECK_TRUE(entity_set.contains(entity), "Entity in set");
				}

				{SCOPE_SECTION("Iterate exact match");
					std::set<ECS::Entity> entity_set;
					double sum_double = 0.0;
					float sum_float   = 0.0f;
					int sum_int       = 0;

					storage.foreach([&](ECS::Entity& p_entity, double& p_double, float& p_float, bool& p_int)
					{
						sum_double += p_double;
						sum_float  += p_float;
						sum_int    += p_int;
						entity_set.insert(p_entity);
					});

					for (const auto& entity : entities)
						CHECK_TRUE(entity_set.contains(entity), "Entity in set");
				}

				{SCOPE_SECTION("Iterate partial match");
					std::set<ECS::Entity> entity_set;
					double sum_double = 0.0;
					float sum_float   = 0.0f;

					storage.foreach([&](ECS::Entity& p_entity, float& p_float, double& p_double)
					{
						sum_double += p_double;
						sum_float  += p_float;
						entity_set.insert(p_entity);
					});

					for (const auto& entity : entities)
						CHECK_TRUE(entity_set.contains(entity), "Entity in set");
				}

				{SCOPE_SECTION("Clear storage")
					for (const auto& entity : entities)
						storage.delete_entity(entity);
					entities.clear();

					{// Iterate empty after delete
						double sum_double = 0.0;
						float sum_float   = 0.0f;
						int sum_int       = 0;
						size_t count      = 0;

						storage.foreach([&](ECS::Entity& p_entity, double& p_double, float& p_float, bool& p_int)
						{
							sum_double += p_double;
							sum_float  += p_float;
							sum_int    += p_int;
							count++;
						});
						CHECK_EQUAL(count, 0, "No iteration after clear");
					}
				}
			}
		}
	}
} // namespace Test
DISABLE_WARNING_POP