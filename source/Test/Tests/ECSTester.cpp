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
DISABLE_WARNING_UNUSED_VARIABLE  // Required to stop variables being destroyed before they are used in tests.
DISABLE_WARNING_UNUSED_PARAMETER // Required until ECS::Storage::count_components<ComponentTypes> is implemented

namespace Test
{
	size_t ECSTester::countEntities(ECS::Storage& pStorage)
	{
		size_t count = 0;
		pStorage.foreach([&count](const ECS::Entity& p_entity){ count++;});
		return count;
	}

	// Tests if any MemoryCorrectnessErrors occurred and if the number of items alive matches pAliveCountExpected
	void ECSTester::runMemoryTests(const std::string& pTestName, const size_t& pAliveCountExpected)
	{
		emplace_unit_test({MemoryCorrectnessItem::count_errors() == 0, pTestName, "Mem Errors found"});
		emplace_unit_test({MemoryCorrectnessItem::count_alive() == pAliveCountExpected, pTestName + " memory test", std::format("Expected {} MemItems alive, was {}", pAliveCountExpected, MemoryCorrectnessItem::count_alive())});
	}


	void ECSTester::run_performance_tests()
	{}

	void ECSTester::run_unit_tests()
	{
		{ // add_entity
			{ // add_entity basic
				MemoryCorrectnessItem::reset(); // Reset before starting new tests
				ECS::Storage storage;
				const float floatComponent = 42.f;
				const double doubleComponent = 13.0;

				emplace_unit_test({countEntities(storage) == 0, "AddEntity - Start Empty", "Storage should initialise empty"});
				runMemoryTests("AddEntity - Start empty", 0);

				storage.add_entity(floatComponent);
				emplace_unit_test({countEntities(storage) == 1, "AddEntity - Add single component entity", "Storage should contain 1 entity"});

				storage.add_entity(doubleComponent);
				emplace_unit_test({countEntities(storage) == 2, "AddEntity - Add another single component entity", "Storage should contain 2 entities"});

				storage.add_entity(doubleComponent, floatComponent);
				emplace_unit_test({countEntities(storage) == 3, "AddEntity - Add another entity with both component types", "Storage should contain 3 entities"});
			}
			{
				MemoryCorrectnessItem::reset(); // Reset before starting new tests
				ECS::Storage storage;
				MemoryCorrectnessItem comp;

				storage.add_entity(comp);
				emplace_unit_test({countEntities(storage) == 1, "AddEntity - Add 1 entity by copy", "Storage should contain 1 entity"});
				runMemoryTests("AddEntity - Add 1 entity by copy", 2);

				float componentFloat = 13;
				storage.add_entity(componentFloat);
				emplace_unit_test({countEntities(storage) == 2, "AddEntity - Add second entity new component", "Storage doesnt contain 2 entities"});
				emplace_unit_test({MemoryCorrectnessItem::count_errors() == 0, "AddEntity - Add second entity new component", "Memory correctness errors found"});

				storage.add_entity(MemoryCorrectnessItem());
				emplace_unit_test({countEntities(storage) == 3, "AddEntity - Add by rvalue", "Storage doesnt contain 3 entities"});
				runMemoryTests("AddEntity - Add by rvalue", 3);

				for (size_t i = 0; i < 100; i++)
					storage.add_entity(MemoryCorrectnessItem());

				emplace_unit_test({countEntities(storage) == 103, "AddEntity - Add 100 more entities", "Storage should contain 103 entities"});
				runMemoryTests("AddEntity - Add by rvalue", 103);
			}
		}
		{ // delete_entity - These rely on add_entity working correctly.
			{
				MemoryCorrectnessItem::reset(); // Reset before starting new tests
				ECS::Storage storage;
				auto item = MemoryCorrectnessItem();

				auto ent = storage.add_entity(item);
				storage.delete_entity(ent);

				emplace_unit_test({countEntities(storage) == 0, "delete_entity - Add 1 entity by copy then delete", "Storage should contain 0 entities"});
				runMemoryTests("delete_entity - Add 1 entity by copy then delete", 1);
			}
			{
				MemoryCorrectnessItem::reset(); // Reset before starting new tests
				ECS::Storage storage;
				auto ent = storage.add_entity(MemoryCorrectnessItem());
				storage.delete_entity(ent);

				emplace_unit_test({countEntities(storage) == 0, "delete_entity - Add 1 entity by rvalue then delete", "Storage should contain 0 entities"});
				runMemoryTests("delete_entity - Add 1 entity by rvalue then delete", 0);
			}
			{
				MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
				ECS::Storage storage;
				storage.add_entity(MemoryCorrectnessItem());
			}
			runMemoryTests("delete_entity - Storage out of scope cleanup", 0); // Dangling memory check

			{// Add 3 delete back to front
				MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
				ECS::Storage storage;
				auto frontEnt  = storage.add_entity(MemoryCorrectnessItem());
				auto middleEnt = storage.add_entity(MemoryCorrectnessItem());
				auto backEnt   = storage.add_entity(MemoryCorrectnessItem());

				storage.delete_entity(backEnt);
				emplace_unit_test({countEntities(storage) == 2, "delete_entity - Delete 3 back-to-front first delete", "Storage should contain 2 entities"});
				runMemoryTests("delete_entity - Delete 3 back-to-front first delete", 2);

				storage.delete_entity(middleEnt);
				emplace_unit_test({countEntities(storage) == 1, "delete_entity - Delete 3 back-to-front second delete", "Storage should contain 1 entity"});
				runMemoryTests("delete_entity - Delete 3 back-to-front second delete", 1);

				storage.delete_entity(frontEnt);
				emplace_unit_test({countEntities(storage) == 0, "delete_entity - Delete 3 back-to-front third delete", "Storage should contain 0 entities"});
				runMemoryTests("delete_entity - Delete 3 back-to-front third delete", 0);
			}
			{// Add 3 delete front to back
				MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
				ECS::Storage storage;
				auto frontEnt  = storage.add_entity(MemoryCorrectnessItem());
				auto middleEnt = storage.add_entity(MemoryCorrectnessItem());
				auto backEnt   = storage.add_entity(MemoryCorrectnessItem());

				storage.delete_entity(frontEnt);
				emplace_unit_test({countEntities(storage) == 2, "delete_entity - Delete 3 front-to-back first delete", "Storage should contain 2 entities"});
				runMemoryTests("delete_entity - Delete 3 front-to-back first delete", 2);

				storage.delete_entity(middleEnt);
				emplace_unit_test({countEntities(storage) == 1, "delete_entity - Delete 3 front-to-back second delete", "Storage should contain 1 entity"});
				runMemoryTests("delete_entity - Delete 3 front-to-back second delete", 1);

				storage.delete_entity(backEnt);
				emplace_unit_test({countEntities(storage) == 0, "delete_entity - Delete 3 front-to-back third delete", "Storage should contain 0 entities"});
				runMemoryTests("delete_entity - Delete 3 front-to-back third delete", 0);
			}
			{// Add 3 delete middle -> front -> back
				MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
				ECS::Storage storage;
				auto frontEnt  = storage.add_entity(MemoryCorrectnessItem());
				auto middleEnt = storage.add_entity(MemoryCorrectnessItem());
				auto backEnt   = storage.add_entity(MemoryCorrectnessItem());

				storage.delete_entity(middleEnt);
				emplace_unit_test({countEntities(storage) == 2, "delete_entity - Add 3, delete middle -> front -> back", "Storage should contain 2 entities"});
				runMemoryTests("delete_entity - Add 3, delete middle -> front -> back", 2);

				storage.delete_entity(frontEnt);
				emplace_unit_test({countEntities(storage) == 1, "delete_entity - Add 3, delete middle -> front -> back", "Storage should contain 1 entity"});
				runMemoryTests("delete_entity - Add 3, delete middle -> front -> back", 1);

				storage.delete_entity(backEnt);
				emplace_unit_test({countEntities(storage) == 0, "delete_entity - Add 3, delete middle -> front -> back", "Storage should contain 0 entities"});
				runMemoryTests("delete_entity - Add 3, delete middle -> front -> back", 0);
			}
			{// Add 100 delete 100 in random order
				MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
				ECS::Storage storage;

				std::vector<ECS::Entity> entities;
				for (size_t i = 0; i < 100; i++)
					entities.push_back(storage.add_entity(MemoryCorrectnessItem()));

				// shuffle the order of entities
				auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
				auto e = std::default_random_engine(seed);
				std::shuffle(entities.begin(), entities.end(), e);

				for (auto& ent : entities)
					storage.delete_entity(ent);

				emplace_unit_test({countEntities(storage) == 0, "delete_entity - Delete 100 entities in random order", "Storage should contain 0 entities"});
				runMemoryTests("delete_entity - Delete 100 entities in random order", 0);
			}
			{ // Overwrite memory test
				MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
				ECS::Storage storage;

				auto ent = storage.add_entity(MemoryCorrectnessItem());
				storage.delete_entity(ent);
				storage.add_entity(MemoryCorrectnessItem());

				emplace_unit_test({countEntities(storage) == 1, "delete_entity - overwrite Add -> Delete -> Add", "Storage should contain 1 entity"});
				runMemoryTests("delete_entity - overwrite Add -> Delete -> Add", 1);
			}
			{ // Overwrite memory test 100
				MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
				ECS::Storage storage;

				std::vector<ECS::Entity> entities;
				for (size_t i = 0; i < 100; i++)
					entities.push_back(storage.add_entity(MemoryCorrectnessItem()));
				for (auto& ent : entities)
					storage.delete_entity(ent);
				for (size_t i = 0; i < 100; i++)
					entities.push_back(storage.add_entity(MemoryCorrectnessItem()));

				emplace_unit_test({countEntities(storage) == 100, "delete_entity - Add 100, Delete 100, Add 100", "Storage should contain 0 entities"});
				runMemoryTests("delete_entity - Add 100, Delete 100, Add 100", 100);
			}
		}
		{ // get_component tests
			{
				ECS::Storage storage;
				auto entity = storage.add_entity(42.0);
				emplace_unit_test({storage.get_component<double>(entity) == 42.0,  "get_component - single component entity", "Incorrect value returned for single component (double)"});

				// Signature variations
				emplace_unit_test({storage.get_component<double&>(entity) == 42.0, "get_component - non-const & get", "Incorrect value returned for single component (double)"});
				emplace_unit_test({storage.get_component<const double&>(entity) == 42.0, "get_component - const & get", "Incorrect value returned for single component (double)"});

				// std::decay doesnt work on * so the below dont compile for now.
				// emplace_unit_test({storage.get_component<double*>(entity) == 42.0, "get_component - single component* entity", "Incorrect value returned for single component (double)"});
				// emplace_unit_test({storage.get_component<const double*>(entity) == 42.0, "get_component - single 'const component*' entity", "Incorrect value returned for single component (double)"});
			}
			{ // This series of tests reuses the same storage instance
				ECS::Storage storage;
				{ // get front middle and end component.
					auto entity = storage.add_entity(1.0, 2.f, true);
					emplace_unit_test({storage.get_component<double>(entity) == 1.0, "get_component - 3 component entity 1", "Incorrect value returned for front (double) component"});
					emplace_unit_test({storage.get_component<float>(entity) == 2.0f, "get_component - 3 component entity 2", "Incorrect value returned for middle (float) component"});
					emplace_unit_test({storage.get_component<bool>(entity) == true,  "get_component - 3 component entity 3", "Incorrect value returned for back (bool) component"});
				}
				{ // Add an entity with the same component makeup but in a reverse order.
					auto entityreverse = storage.add_entity(false, 1.f, 2.0);
					emplace_unit_test({storage.get_component<double>(entityreverse) == 2.0, "get_component - 3 component entity - same types, reverse order 1", "Incorrect value returned for double component"});
					emplace_unit_test({storage.get_component<float>(entityreverse) == 1.0f, "get_component - 3 component entity - same types, reverse order 2", "Incorrect value returned for float component"});
					emplace_unit_test({storage.get_component<bool>(entityreverse) == false, "get_component - 3 component entity - same types, reverse order 3", "Incorrect value returned for bool component"});
				}
				{ // Add an entity with the same component makeup but in a new order.
					auto entityNew = storage.add_entity(13.f, true, 42.0);
					emplace_unit_test({storage.get_component<double>(entityNew) == 42.0, "get_component - 3 component entity - same types, new order 1", "Incorrect value returned for double component"});
					emplace_unit_test({storage.get_component<float>(entityNew) == 13.0f, "get_component - 3 component entity - same types, new order 2", "Incorrect value returned for float component"});
					emplace_unit_test({storage.get_component<bool>(entityNew) == true,   "get_component - 3 component entity - same types, new order 3", "Incorrect value returned for bool component"});
				}
				{ // Add an entity with a new combination of components.
					auto entityNew = storage.add_entity('G');
					emplace_unit_test({storage.get_component<char>(entityNew) == 'G', "get_component - new component combination", "Incorrect value returned for char component"});
				}

				{// Data type limits - setting as many bits as possible
					constexpr double maxDouble = std::numeric_limits<double>::max();
					constexpr double minDouble = std::numeric_limits<double>::min();

					auto entityMaxDouble1 = storage.add_entity(maxDouble);
					auto entityMinDouble1 = storage.add_entity(minDouble);
					auto entityMaxDouble2 = storage.add_entity(maxDouble);
					auto entityMaxDouble3 = storage.add_entity(maxDouble);
					auto entityMinDouble2 = storage.add_entity(minDouble);
					auto entityMinDouble3 = storage.add_entity(minDouble);
					auto entityMinDouble4 = storage.add_entity(minDouble);
					auto entityMaxDouble4 = storage.add_entity(maxDouble);

					emplace_unit_test({storage.get_component<double>(entityMaxDouble1) == maxDouble, "get_component - Data type limits 1", "Incorrect value returned for max double component"});
					emplace_unit_test({storage.get_component<double>(entityMaxDouble2) == maxDouble, "get_component - Data type limits 2", "Incorrect value returned for max double component"});
					emplace_unit_test({storage.get_component<double>(entityMaxDouble3) == maxDouble, "get_component - Data type limits 3", "Incorrect value returned for max double component"});
					emplace_unit_test({storage.get_component<double>(entityMaxDouble4) == maxDouble, "get_component - Data type limits 4", "Incorrect value returned for max double component"});
					emplace_unit_test({storage.get_component<double>(entityMinDouble1) == minDouble, "get_component - Data type limits 1", "Incorrect value returned for min double component"});
					emplace_unit_test({storage.get_component<double>(entityMinDouble2) == minDouble, "get_component - Data type limits 2", "Incorrect value returned for min double component"});
					emplace_unit_test({storage.get_component<double>(entityMinDouble3) == minDouble, "get_component - Data type limits 3", "Incorrect value returned for min double component"});
					emplace_unit_test({storage.get_component<double>(entityMinDouble4) == minDouble, "get_component - Data type limits 4", "Incorrect value returned for min double component"});
				}
				{ // get_component MemoryCorrectness
					MemoryCorrectnessItem::reset();
					auto memCorrectEntity = storage.add_entity(MemoryCorrectnessItem());

					auto& compRef = storage.get_component<MemoryCorrectnessItem>(memCorrectEntity);
					runMemoryTests("get_component - get by reference no new items", 1);

					// No &, copy comp
					auto compCopy = storage.get_component<MemoryCorrectnessItem>(memCorrectEntity);
					runMemoryTests("get_component - get by copy 1 new item", 2);
				}
				runMemoryTests("get_component - copy out of scope 1 remaining inside storage", 1);
			}
		}
		{ // get_component_mutable tests
			{ // edit component value
				ECS::Storage storage;

				{ // Add -> get -> set -> check
					auto entity     = storage.add_entity(42.0);
					auto& comp      = storage.get_component_mutable<double>(entity);
					comp            = 69.0;
					auto& compAgain = storage.get_component<double>(entity);
					emplace_unit_test({compAgain == 69.0, "get_component_mutable - get and set", "Assigned value not correct"});

					storage.get_component_mutable<double>(entity) += 10.0;
					emplace_unit_test({compAgain == 79.0, "get_component_mutable - get and set one liner", "Assigned value not correct"});
				}
				{ // Add second ent to same archetype -> get -> set -> check
					auto entity = storage.add_entity(27.0);
					storage.get_component_mutable<double>(entity) += 3.0;
					emplace_unit_test({storage.get_component<double>(entity) == 30.0, "get_component_mutable - get and set to same archetype", "Assigned value not correct"});
				}
				{ // Add to new archetype -> get -> set -> check
					auto entity = storage.add_entity(27.0, 49.f);
					storage.get_component_mutable<double>(entity) += 3.0;
					emplace_unit_test({storage.get_component<double>(entity) == 30.0, "get_component_mutable - Add to new archetype -> get -> set -> check", "Assigned value not correct"});

					storage.get_component_mutable<float>(entity) += 1.0f;
					emplace_unit_test({storage.get_component<float>(entity) == 50.0f, "get_component_mutable - Add to new archetype -> get -> set -> check 2", "Assigned value not correct"});
				}
				{ // Add 3 component entity and edit in reverse order in memory
					auto entity = storage.add_entity(1.0, 2.f, 3);
					storage.get_component_mutable<int>(entity) += 1;
					emplace_unit_test({storage.get_component<int>(entity) == 4, "get_component_mutable - Add 3 component entity -> edit each comp in reverse 1", "Assigned value not correct"});

					storage.get_component_mutable<float>(entity) += 19.0f;
					emplace_unit_test({storage.get_component<float>(entity) == 21.f, "get_component_mutable - Add 3 component entity -> edit each comp in reverse 2", "Assigned value not correct"});

					storage.get_component_mutable<double>(entity) += 13.0;
					emplace_unit_test({storage.get_component<double>(entity) == 14.0, "get_component_mutable - Add 3 component entity -> edit each comp in reverse 3", "Assigned value not correct"});
				}
				{ // get_component_mutable MemoryCorrectness
					MemoryCorrectnessItem::reset();
					auto memCorrectEntity = storage.add_entity(MemoryCorrectnessItem());

					auto& compRef = storage.get_component_mutable<MemoryCorrectnessItem>(memCorrectEntity);
					runMemoryTests("get_component_mutable - get by reference no new items", 1);

					// No &, copy comp
					auto compCopy = storage.get_component_mutable<MemoryCorrectnessItem>(memCorrectEntity);
					runMemoryTests("get_component_mutable - get by copy 1 new item", 2);
				}
				runMemoryTests("get_component_mutable - copy out of scope 1 remaining inside storage", 1);
			}
		}
		{// hasComponent tests
			ECS::Storage storage;
			{ // HasComponents exact match multiple types
				const auto entity  = storage.add_entity(1.0, 2.f, true);
				auto has_components = storage.has_components<double, float, bool>(entity);
				emplace_unit_test({has_components == true, "has_components - exact match multiple types" , "has_components: incorrect"});
			}
			{ // HasComponents exact match multiple types different order
				const auto entity  = storage.add_entity(1.0, 2.f, true);
				auto has_components = storage.has_components<bool, float, double>(entity);
				emplace_unit_test({has_components == true, "has_components - exact match different order multiple types" , "has_components: incorrect"});
			}
			{ // HasComponents exact match single type multiple component
				const auto entity  = storage.add_entity(1.0, 2.f, true);
				auto has_components = storage.has_components<float>(entity);
				emplace_unit_test({has_components == true, "has_components - single type match from multiple component middle" , "has_components: incorrect"});
			}
			{ // HasComponents exact match single type single component
				const auto entity  = storage.add_entity(1.0);
				auto has_components = storage.has_components<double>(entity);
				emplace_unit_test({has_components == true, "has_components - exact match single type single component" , "has_components: incorrect"});
			}
			{ // HasComponents subset match
				const auto entity  = storage.add_entity(1.0, 2.f, true);
				auto has_components = storage.has_components<double, bool>(entity);
				emplace_unit_test({has_components == true, "has_components - subset match" , "has_components: incorrect"});
			}
			{ // HasComponents subset match different order
				const auto entity  = storage.add_entity(1.0, 2.f, true);
				auto has_components = storage.has_components<bool, double>(entity);
				emplace_unit_test({has_components == true, "has_components - subset match different order" , "has_components: incorrect"});
			}
			{ //  HasComponents subset match single type
				const auto entity  = storage.add_entity(1.0, 2.f, true);
				auto has_components = storage.has_components<double>(entity);
				emplace_unit_test({has_components == true, "has_components - subset match single type" , "has_components: incorrect"});
			}
			{ //  HasComponents no match single type
				const auto entity  = storage.add_entity(1.0, 2.f, true);
				auto has_components = storage.has_components<std::string>(entity);
				emplace_unit_test({has_components == false, "has_components - no match single type" , "has_components: incorrect"});
			}
			{ //  HasComponents no match multiple types
				const auto entity  = storage.add_entity(1.0, 2.f, true);
				auto has_components = storage.has_components<std::string, size_t>(entity);
				emplace_unit_test({has_components == false, "has_components - no match multiple types" , "has_components: incorrect"});
			}
		}
		{// foreach tests
			ECS::Storage storage;
			storage.add_entity(13.69, 1.33f, 2);
			storage.add_entity(13.69, 1.33f, 2);
			storage.add_entity(13.69, 1.33f, 2);

			{ // Exact match and order to archetype
				size_t count = 0;
				storage.foreach ([this, &count](double& p_double, float& pFloat, int& pInt)
				{
					emplace_unit_test({p_double == 13.69, "foreach - Exact match and order to archetype 1", "foreach: Missmatch value"});
					emplace_unit_test({pInt == 2, "foreach - Exact match and order to archetype 2", "foreach: Missmatch value"});
					emplace_unit_test({pFloat == 1.33f, "foreach - Exact match and order to archetype 3", "foreach: Missmatch value"});
					count++;
				});
				emplace_unit_test({count == 3, "foreach - iterate count Exact match and order to archetype", "foreach: Missmatch value"});
			}
			{ // Exact match different order to archetype
				size_t count = 0;
				storage.foreach ([this, &count](float& pFloat, int& pInt, double& p_double)
				{
					emplace_unit_test({p_double == 13.69, "foreach - Exact match different order to archetype 1", "foreach: Missmatch value"});
					emplace_unit_test({pInt == 2, "foreach - Exact match function arguments different order to archetype 2", "foreach: Missmatch value"});
					emplace_unit_test({pFloat == 1.33f, "foreach - Exact match function arguments different order to archetype 3", "foreach: Missmatch value"});
					count++;
				});
				emplace_unit_test({count == 3, "foreach - iterate count Exact match different order to archetype", "foreach: Missmatch value"});
			}
			{ // Subset match same order to archetype
				size_t count = 0;
				storage.foreach ([this, &count](double& p_double, float& pFloat)
				{
					emplace_unit_test({p_double == 13.69, "foreach - subset match same order to archetype 1", "foreach: Missmatch value"});
					emplace_unit_test({pFloat == 1.33f, "foreach - subset match same order to archetype 2", "foreach: Missmatch value"});
					count++;
				});
				emplace_unit_test({count == 3, "foreach - iterate count subset match same order to archetype", "foreach: Missmatch value"});
			}
			{ // Subset match different order to archetype
				size_t count = 0;
				storage.foreach ([this, &count](int& pInt, float& pFloat)
				{
					emplace_unit_test({pInt == 2, "foreach - Subset match different order to archetype 1", "foreach: Missmatch value"});
					emplace_unit_test({pFloat == 1.33f, "foreach - Subset match different order to archetype 2", "foreach: Missmatch value"});
					count++;
				});
				emplace_unit_test({count == 3, "foreach - iterate count - Subset match different order to archetype", "foreach: Missmatch value"});
			}
			{ // Single argument match to archetype
				size_t count = 0;
				storage.foreach ([this, &count](double& p_double)
				{
					emplace_unit_test({p_double == 13.69, "foreach - Single argument match to archetype", "foreach: Missmatch value"});
					count++;
				});
				emplace_unit_test({count == 3, "foreach - iterate count - Single argument match to archetype", "foreach: Missmatch value"});
			}
			{ // Single argument match to archetype - back component
				size_t count = 0;
				storage.foreach ([this, &count](int& pInt)
				{
					emplace_unit_test({pInt == 2, "foreach - Single argument match to archetype - back component", "foreach: Missmatch value"});
					count++;
				});
				emplace_unit_test({count == 3, "foreach - iterate count - Single argument match to archetype - back component", "foreach: Missmatch value"});
			}
			{ // Single argument match to archetype back component
				size_t count = 0;
				storage.foreach ([this, &count](float& pFloat)
				{
					emplace_unit_test({pFloat == 1.33f, "foreach - Single argument match to archetype back component", "foreach: Missmatch value"});
					count++;
				});
				emplace_unit_test({count == 3, "foreach - iterate count - Single argument match to archetype back component", "foreach: Missmatch value"});
			}
			{ // Exact match change data
				size_t count = 0;
				storage.foreach ([&count](double& p_double, float& pFloat, int& pInt)
				{
					p_double += 1.0;
					pFloat += 1.0f;
					pInt += 1;
					count++;
				});
				emplace_unit_test({count == 3, "foreach - iterate count - Exact match change data", "foreach: Missmatch value"});
			}
			{ // Exact match check changed data
				storage.foreach ([this](double& p_double, float& pFloat, int& pInt)
				{
					emplace_unit_test({p_double == 14.69, "foreach - Exact match check changed data", "foreach: Missmatch value"});
					emplace_unit_test({pInt == 3, "foreach - Exact match check changed data", "foreach: Missmatch value"});
					emplace_unit_test({pFloat == 2.33f, "foreach - Exact match check changed data", "foreach: Missmatch value"});
				});
			}
			{ // Add a new entity to a new archetype
				storage.add_entity(13.0);
				size_t count = 0;
				storage.foreach ([&count](double& p_double){ count++; });
				emplace_unit_test({count == 4, "foreach - iterate a component inside two archetypes", "Expected 4 components of type double"});
			}
		}
		{ // foreach with Entity
			ECS::Storage storage;
			std::vector<ECS::Entity> entities;

			{// Iterate empty before add
				size_t count = 0;
				storage.foreach ([&count](ECS::Entity& p_entity, double& p_double, float& pFloat, bool& pInt) { count++; });
				emplace_unit_test({count == 0, "foreach(Entity)", "Entity count should be 0 before any add"});
			}

			for (size_t i = 0; i < 12; i++)
				entities.push_back(storage.add_entity(1.0, 2.f, true));

			{ // Iterate exact match archetype and count the same unique set of entities returned
				std::set<ECS::Entity> entitySet;
				storage.foreach ([&entitySet](ECS::Entity& p_entity, double& p_double, float& pFloat, bool& pInt)
				{
					entitySet.insert(p_entity);
				});

				emplace_unit_test({entitySet.size() == 12, "foreach(Entity)", "Set size should match the 12 entities added"});
				for (const auto& entity : entities)
					emplace_unit_test({entitySet.contains(entity), "foreach(Entity)", "Entity missing from foreach"});
			}
			{ // Iterate partial match archetype and count the same unique set of entities returned
				std::set<ECS::Entity> entitySet;
				storage.foreach ([&entitySet](ECS::Entity& p_entity, float& pFloat, double& p_double)
				{
					entitySet.insert(p_entity);
				});

				emplace_unit_test({entitySet.size() == 12, "foreach(Entity)", "Set size should match the 12 entities added"});
				for (const auto& entity : entities)
					emplace_unit_test({entitySet.contains(entity), "foreach(Entity)", "Entity missing from foreach"});
			}

			// Remove all the entities in storage
			for (const auto& entity : entities)
				storage.delete_entity(entity);
			entities.clear();

			{// Iterate empty after delete
				size_t count = 0;
				storage.foreach ([&count](ECS::Entity& p_entity, double& p_double, float& pFloat, bool& pInt) { count++; });
				emplace_unit_test({count == 0, "foreach(Entity)", "Entity count should be 0 after all entities deleted"});
			}
		}
	}
} // namespace Test
DISABLE_WARNING_POP