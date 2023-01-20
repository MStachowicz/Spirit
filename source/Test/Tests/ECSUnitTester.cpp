#include "ECSUnitTester.hpp"

// TEST
#include "MemoryCorrectnessItem.hpp"

// ECS
#include "Storage.hpp"

// UTILITY
#include "Logger.hpp"

// STD
#include <set>
#include <algorithm>    // std::shuffle
#include <vector>       // std::vector
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock

namespace Test
{
    size_t ECSUnitTester::countEntities(ECS::Storage& pStorage)
    {
        size_t count = 0;
        pStorage.foreach([&count](const ECS::Entity& pEntity){ count++;});
        return count;
    }

    // Tests if any MemoryCorrectnessErrors occurred and if the number of items alive matches pAliveCountExpected
    void ECSUnitTester::runMemoryTests(const std::string& pTestName, const size_t& pAliveCountExpected)
    {
        runTest({MemoryCorrectnessItem::countErrors() == 0, pTestName, "Mem Errors found"});
        runTest({MemoryCorrectnessItem::countAlive() == pAliveCountExpected, pTestName + " memory test", std::format("Expected {} MemItems alive, was {}", pAliveCountExpected, MemoryCorrectnessItem::countAlive())});
    }

    void ECSUnitTester::runAllTests()
    {
        { // addEntity
            { // addEntity basic
                MemoryCorrectnessItem::reset(); // Reset before starting new tests
                ECS::Storage storage;
                const float floatComponent = 42.f;
                const double doubleComponent = 13.0;

                runTest({countEntities(storage) == 0, "AddEntity - Start Empty", "Storage should initialise empty"});
                runMemoryTests("AddEntity - Start empty", 0);

                storage.addEntity(floatComponent);
                runTest({countEntities(storage) == 1, "AddEntity - Add single component entity", "Storage should contain 1 entity"});

                storage.addEntity(doubleComponent);
                runTest({countEntities(storage) == 2, "AddEntity - Add another single component entity", "Storage should contain 2 entities"});

                storage.addEntity(doubleComponent, floatComponent);
                runTest({countEntities(storage) == 3, "AddEntity - Add another entity with both component types", "Storage should contain 3 entities"});
            }
            {
                MemoryCorrectnessItem::reset(); // Reset before starting new tests
                ECS::Storage storage;
                MemoryCorrectnessItem comp;

                storage.addEntity(comp);
                runTest({countEntities(storage) == 1, "AddEntity - Add 1 entity by copy", "Storage should contain 1 entity"});
                runMemoryTests("AddEntity - Add 1 entity by copy", 2);

                float componentFloat = 13;
                storage.addEntity(componentFloat);
                runTest({countEntities(storage) == 2, "AddEntity - Add second entity new component", "Storage doesnt contain 2 entities"});
                runTest({MemoryCorrectnessItem::countErrors() == 0, "AddEntity - Add second entity new component", "Memory correctness errors found"});

                storage.addEntity(MemoryCorrectnessItem());
                runTest({countEntities(storage) == 3, "AddEntity - Add by rvalue", "Storage doesnt contain 3 entities"});
                runMemoryTests("AddEntity - Add by rvalue", 3);

                for (size_t i = 0; i < 100; i++)
                    storage.addEntity(MemoryCorrectnessItem());

                runTest({countEntities(storage) == 103, "AddEntity - Add 100 more entities", "Storage should contain 103 entities"});
                runMemoryTests("AddEntity - Add by rvalue", 103);
            }
        }
        { // deleteEntity - These rely on addEntity working correctly.
            {
                MemoryCorrectnessItem::reset(); // Reset before starting new tests
                ECS::Storage storage;
                auto item = MemoryCorrectnessItem();

                auto ent = storage.addEntity(item);
                storage.deleteEntity(ent);

                runTest({countEntities(storage) == 0, "deleteEntity - Add 1 entity by copy then delete", "Storage should contain 0 entities"});
                runMemoryTests("deleteEntity - Add 1 entity by copy then delete", 1);
            }
            {
                MemoryCorrectnessItem::reset(); // Reset before starting new tests
                ECS::Storage storage;
                auto ent = storage.addEntity(MemoryCorrectnessItem());
                storage.deleteEntity(ent);

                runTest({countEntities(storage) == 0, "deleteEntity - Add 1 entity by rvalue then delete", "Storage should contain 0 entities"});
                runMemoryTests("deleteEntity - Add 1 entity by rvalue then delete", 0);
            }
            {
                MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
                ECS::Storage storage;
                auto ent = storage.addEntity(MemoryCorrectnessItem());
            }
            runMemoryTests("deleteEntity - Storage out of scope cleanup", 0); // Dangling memory check

            {// Add 3 delete back to front
                MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
                ECS::Storage storage;
                auto frontEnt  = storage.addEntity(MemoryCorrectnessItem());
                auto middleEnt = storage.addEntity(MemoryCorrectnessItem());
                auto backEnt   = storage.addEntity(MemoryCorrectnessItem());

                storage.deleteEntity(backEnt);
                runTest({countEntities(storage) == 2, "deleteEntity - Delete 3 back-to-front first delete", "Storage should contain 2 entities"});
                runMemoryTests("deleteEntity - Delete 3 back-to-front first delete", 2);

                storage.deleteEntity(middleEnt);
                runTest({countEntities(storage) == 1, "deleteEntity - Delete 3 back-to-front second delete", "Storage should contain 1 entity"});
                runMemoryTests("deleteEntity - Delete 3 back-to-front second delete", 1);

                storage.deleteEntity(frontEnt);
                runTest({countEntities(storage) == 0, "deleteEntity - Delete 3 back-to-front third delete", "Storage should contain 0 entities"});
                runMemoryTests("deleteEntity - Delete 3 back-to-front third delete", 0);
            }
            {// Add 3 delete front to back
                MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
                ECS::Storage storage;
                auto frontEnt  = storage.addEntity(MemoryCorrectnessItem());
                auto middleEnt = storage.addEntity(MemoryCorrectnessItem());
                auto backEnt   = storage.addEntity(MemoryCorrectnessItem());

                storage.deleteEntity(frontEnt);
                runTest({countEntities(storage) == 2, "deleteEntity - Delete 3 front-to-back first delete", "Storage should contain 2 entities"});
                runMemoryTests("deleteEntity - Delete 3 front-to-back first delete", 2);

                storage.deleteEntity(middleEnt);
                runTest({countEntities(storage) == 1, "deleteEntity - Delete 3 front-to-back second delete", "Storage should contain 1 entity"});
                runMemoryTests("deleteEntity - Delete 3 front-to-back second delete", 1);

                storage.deleteEntity(backEnt);
                runTest({countEntities(storage) == 0, "deleteEntity - Delete 3 front-to-back third delete", "Storage should contain 0 entities"});
                runMemoryTests("deleteEntity - Delete 3 front-to-back third delete", 0);
            }
            {// Add 3 delete middle -> front -> back
                MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
                ECS::Storage storage;
                auto frontEnt  = storage.addEntity(MemoryCorrectnessItem());
                auto middleEnt = storage.addEntity(MemoryCorrectnessItem());
                auto backEnt   = storage.addEntity(MemoryCorrectnessItem());

                storage.deleteEntity(middleEnt);
                runTest({countEntities(storage) == 2, "deleteEntity - Add 3, delete middle -> front -> back", "Storage should contain 2 entities"});
                runMemoryTests("deleteEntity - Add 3, delete middle -> front -> back", 2);

                storage.deleteEntity(frontEnt);
                runTest({countEntities(storage) == 1, "deleteEntity - Add 3, delete middle -> front -> back", "Storage should contain 1 entity"});
                runMemoryTests("deleteEntity - Add 3, delete middle -> front -> back", 1);

                storage.deleteEntity(backEnt);
                runTest({countEntities(storage) == 0, "deleteEntity - Add 3, delete middle -> front -> back", "Storage should contain 0 entities"});
                runMemoryTests("deleteEntity - Add 3, delete middle -> front -> back", 0);
            }
            {// Add 100 delete 100 in random order
                MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
                ECS::Storage storage;

                std::vector<ECS::Entity> entities;
                for (size_t i = 0; i < 100; i++)
                    entities.push_back(storage.addEntity(MemoryCorrectnessItem()));

                // shuffle the order of entities
                auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
                auto e = std::default_random_engine(seed);
                std::shuffle(entities.begin(), entities.end(), e);

                for (auto& ent : entities)
                    storage.deleteEntity(ent);

                runTest({countEntities(storage) == 0, "deleteEntity - Delete 100 entities in random order", "Storage should contain 0 entities"});
                runMemoryTests("deleteEntity - Delete 100 entities in random order", 0);
            }
            { // Overwrite memory test
                MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
                ECS::Storage storage;

                auto ent = storage.addEntity(MemoryCorrectnessItem());
                storage.deleteEntity(ent);
                storage.addEntity(MemoryCorrectnessItem());

                runTest({countEntities(storage) == 1, "deleteEntity - overwrite Add -> Delete -> Add", "Storage should contain 1 entity"});
                runMemoryTests("deleteEntity - overwrite Add -> Delete -> Add", 1);
            }
            { // Overwrite memory test 100
                MemoryCorrectnessItem::reset(); // Reset to not invalidate next MemoryCorrectness checks
                ECS::Storage storage;

                std::vector<ECS::Entity> entities;
                for (size_t i = 0; i < 100; i++)
                    entities.push_back(storage.addEntity(MemoryCorrectnessItem()));
                for (auto& ent : entities)
                    storage.deleteEntity(ent);
                for (size_t i = 0; i < 100; i++)
                    entities.push_back(storage.addEntity(MemoryCorrectnessItem()));

                runTest({countEntities(storage) == 100, "deleteEntity - Add 100, Delete 100, Add 100", "Storage should contain 0 entities"});
                runMemoryTests("deleteEntity - Add 100, Delete 100, Add 100", 100);
            }
        }
        { // getComponent tests
            {
                ECS::Storage storage;
                auto entity = storage.addEntity(42.0);
                runTest({storage.getComponent<double>(entity) == 42.0,  "getComponent - single component entity", "Incorrect value returned for single component (double)"});

                // Signature variations
                runTest({storage.getComponent<double&>(entity) == 42.0, "getComponent - non-const & get", "Incorrect value returned for single component (double)"});
                runTest({storage.getComponent<const double&>(entity) == 42.0, "getComponent - const & get", "Incorrect value returned for single component (double)"});

                // std::decay doesnt work on * so the below dont compile for now.
                // runTest({storage.getComponent<double*>(entity) == 42.0, "getComponent - single component* entity", "Incorrect value returned for single component (double)"});
                // runTest({storage.getComponent<const double*>(entity) == 42.0, "getComponent - single 'const component*' entity", "Incorrect value returned for single component (double)"});
            }
            { // This series of tests reuses the same storage instance
                ECS::Storage storage;
                { // get front middle and end component.
                    auto entity = storage.addEntity(1.0, 2.f, true);
                    runTest({storage.getComponent<double>(entity) == 1.0, "getComponent - 3 component entity 1", "Incorrect value returned for front (double) component"});
                    runTest({storage.getComponent<float>(entity) == 2.0f, "getComponent - 3 component entity 2", "Incorrect value returned for middle (float) component"});
                    runTest({storage.getComponent<bool>(entity) == true,  "getComponent - 3 component entity 3", "Incorrect value returned for back (bool) component"});
                }
                { // Add an entity with the same component makeup but in a reverse order.
                    auto entityreverse = storage.addEntity(false, 1.f, 2.0);
                    runTest({storage.getComponent<double>(entityreverse) == 2.0, "getComponent - 3 component entity - same types, reverse order 1", "Incorrect value returned for double component"});
                    runTest({storage.getComponent<float>(entityreverse) == 1.0f, "getComponent - 3 component entity - same types, reverse order 2", "Incorrect value returned for float component"});
                    runTest({storage.getComponent<bool>(entityreverse) == false, "getComponent - 3 component entity - same types, reverse order 3", "Incorrect value returned for bool component"});
                }
                { // Add an entity with the same component makeup but in a new order.
                    auto entityNew = storage.addEntity(13.f, true, 42.0);
                    runTest({storage.getComponent<double>(entityNew) == 42.0, "getComponent - 3 component entity - same types, new order 1", "Incorrect value returned for double component"});
                    runTest({storage.getComponent<float>(entityNew) == 13.0f, "getComponent - 3 component entity - same types, new order 2", "Incorrect value returned for float component"});
                    runTest({storage.getComponent<bool>(entityNew) == true,   "getComponent - 3 component entity - same types, new order 3", "Incorrect value returned for bool component"});
                }
                { // Add an entity with a new combination of components.
                    auto entityNew = storage.addEntity('G');
                    runTest({storage.getComponent<char>(entityNew) == 'G', "getComponent - new component combination", "Incorrect value returned for char component"});
                }

                {// Data type limits - setting as many bits as possible
                    constexpr double maxDouble = std::numeric_limits<double>::max();
                    constexpr double minDouble = std::numeric_limits<double>::min();

                    auto entityMaxDouble1 = storage.addEntity(maxDouble);
                    auto entityMinDouble1 = storage.addEntity(minDouble);
                    auto entityMaxDouble2 = storage.addEntity(maxDouble);
                    auto entityMaxDouble3 = storage.addEntity(maxDouble);
                    auto entityMinDouble2 = storage.addEntity(minDouble);
                    auto entityMinDouble3 = storage.addEntity(minDouble);
                    auto entityMinDouble4 = storage.addEntity(minDouble);
                    auto entityMaxDouble4 = storage.addEntity(maxDouble);

                    runTest({storage.getComponent<double>(entityMaxDouble1) == maxDouble, "getComponent - Data type limits 1", "Incorrect value returned for max double component"});
                    runTest({storage.getComponent<double>(entityMaxDouble2) == maxDouble, "getComponent - Data type limits 2", "Incorrect value returned for max double component"});
                    runTest({storage.getComponent<double>(entityMaxDouble3) == maxDouble, "getComponent - Data type limits 3", "Incorrect value returned for max double component"});
                    runTest({storage.getComponent<double>(entityMaxDouble4) == maxDouble, "getComponent - Data type limits 4", "Incorrect value returned for max double component"});
                    runTest({storage.getComponent<double>(entityMinDouble1) == minDouble, "getComponent - Data type limits 1", "Incorrect value returned for min double component"});
                    runTest({storage.getComponent<double>(entityMinDouble2) == minDouble, "getComponent - Data type limits 2", "Incorrect value returned for min double component"});
                    runTest({storage.getComponent<double>(entityMinDouble3) == minDouble, "getComponent - Data type limits 3", "Incorrect value returned for min double component"});
                    runTest({storage.getComponent<double>(entityMinDouble4) == minDouble, "getComponent - Data type limits 4", "Incorrect value returned for min double component"});
                }
                { // getComponent MemoryCorrectness
                    MemoryCorrectnessItem::reset();
                    auto memCorrectEntity = storage.addEntity(MemoryCorrectnessItem());

                    auto& compRef = storage.getComponent<MemoryCorrectnessItem>(memCorrectEntity);
                    runMemoryTests("getComponent - get by reference no new items", 1);

                    // No &, copy comp
                    auto compCopy = storage.getComponent<MemoryCorrectnessItem>(memCorrectEntity);
                    runMemoryTests("getComponent - get by copy 1 new item", 2);
                }
            }
        }
        { // getComponentMutable tests
            { // edit component value
                ECS::Storage storage;

                { // Add -> get -> set -> check
                    auto entity     = storage.addEntity(42.0);
                    auto& comp      = storage.getComponentMutable<double>(entity);
                    comp            = 69.0;
                    auto& compAgain = storage.getComponent<double>(entity);
                    runTest({compAgain == 69.0, "getComponentMutable - get and set", "Assigned value not correct"});

                    storage.getComponentMutable<double>(entity) += 10.0;
                    runTest({compAgain == 79.0, "getComponentMutable - get and set one liner", "Assigned value not correct"});
                }
                { // Add second ent to same archetype -> get -> set -> check
                    auto entity = storage.addEntity(27.0);
                    storage.getComponentMutable<double>(entity) += 3.0;
                    runTest({storage.getComponent<double>(entity) == 30.0, "getComponentMutable - get and set to same archetype", "Assigned value not correct"});
                }
                { // Add to new archetype -> get -> set -> check
                    auto entity = storage.addEntity(27.0, 49.f);
                    storage.getComponentMutable<double>(entity) += 3.0;
                    runTest({storage.getComponent<double>(entity) == 30.0, "getComponentMutable - Add to new archetype -> get -> set -> check", "Assigned value not correct"});

                    storage.getComponentMutable<float>(entity) += 1.0f;
                    runTest({storage.getComponent<float>(entity) == 50.0f, "getComponentMutable - Add to new archetype -> get -> set -> check 2", "Assigned value not correct"});
                }
                { // Add 3 component entity and edit in reverse order in memory
                    auto entity = storage.addEntity(1.0, 2.f, 3);
                    storage.getComponentMutable<int>(entity) += 1;
                    runTest({storage.getComponent<int>(entity) == 4, "getComponentMutable - Add 3 component entity -> edit each comp in reverse 1", "Assigned value not correct"});

                    storage.getComponentMutable<float>(entity) += 19.0f;
                    runTest({storage.getComponent<float>(entity) == 21.f, "getComponentMutable - Add 3 component entity -> edit each comp in reverse 2", "Assigned value not correct"});

                    storage.getComponentMutable<double>(entity) += 13.0;
                    runTest({storage.getComponent<double>(entity) == 14.0, "getComponentMutable - Add 3 component entity -> edit each comp in reverse 3", "Assigned value not correct"});
                }
            }
        }
        {// foreach tests
        }
    }
} // namespace Test




           // { // forEach tests
           //     ECS::Storage storage;
           //     size_t count = 0;
//
           //     const auto entity  = storage.addEntity(13.69, 1.33f, 2);
           //     const auto entity2 = storage.addEntity(13.69, 1.33f, 2);
           //     const auto entity3 = storage.addEntity(13.69, 1.33f, 2);
           //     { // TEST 1: Exact match - same order as archetype.
           //         storage.foreach ([&count](double& pDouble, float& pFloat, int& pInt)
           //         {
           //             runTest({pDouble == 13.69, "foreach: Missmatch value"});
           //             runTest({pInt == 2, "foreach: Missmatch value"});
           //             runTest({pFloat == 1.33f, "foreach: Missmatch value"});
           //             count++;
           //         });
           //         runTest({count == 3, "foreach: Missmatch value"});
           //         count = 0;
           //     }
           //     { // TEST 2: Exact match - different order to archetype.
           //         storage.foreach ([&count](float& pFloat, int& pInt, double& pDouble)
           //         {
           //             runTest({pDouble == 13.69, "foreach: Missmatch value"});
           //             runTest({pInt == 2, "foreach: Missmatch value"});
           //             runTest({pFloat == 1.33f, "foreach: Missmatch value"});
           //             count++;
           //         });
           //         runTest({count == 3, "foreach: Missmatch value"});
           //         count = 0;
           //     }
           //     { // TEST 3: Subset match - same order as archetype.
           //         storage.foreach ([&count](double& pDouble, float& pFloat)
           //         {
           //             runTest({pDouble == 13.69, "foreach: Missmatch value"});
           //             runTest({pFloat == 1.33f, "foreach: Missmatch value"});
           //             count++;
           //         });
           //         runTest({count == 3, "foreach: Missmatch value"});
           //         count = 0;
           //     }
           //     { // TEST 4: Subset match - different order to archetype.
           //         storage.foreach ([&count](int& pInt, float& pFloat)
           //         {
           //             runTest({pInt == 2, "foreach: Missmatch value"});
           //             runTest({pFloat == 1.33f, "foreach: Missmatch value"});
           //             count++;
           //         });
           //         runTest({count == 3, "foreach: Missmatch value"});
           //         count = 0;
           //     }
           //     { // TEST 4: Subset match single type - front of archetype.
           //         storage.foreach ([&count](double& pDouble)
           //         {
           //             runTest({pDouble == 13.69, "foreach: Missmatch value"});
           //             count++;
           //         });
           //         runTest({count == 3, "foreach: Missmatch value"});
           //         count = 0;
           //     }
           //     { // TEST 4: Subset match single type - back of archetype.
           //         storage.foreach ([&count](int& pInt)
           //         {
           //             runTest({pInt == 2, "foreach: Missmatch value"});
           //             count++;
           //         });
           //         runTest({count == 3, "foreach: Missmatch value"});
           //         count = 0;
           //     }
           //     { // TEST 4: Subset match single type - middle of archetype.
           //         storage.foreach ([&count](float& pFloat)
           //                          {
           //                 runTest({pFloat == 1.33f, "foreach: Missmatch value"});
           //         count++; });
           //         runTest({count == 3, "foreach: Missmatch value"});
           //         count = 0;
           //     }
           //     { // TEST 1: Exact match change ECS data.
           //         storage.foreach ([&count](double& pDouble, float& pFloat, int& pInt)
           //         {
           //             pDouble += 1.0;
           //             pInt += 1;
           //             pFloat += 1.0f;
           //             count++;
           //         });
           //         runTest({count == 3, "foreach: Missmatch value"});
           //         count = 0;
//
           //         storage.foreach ([&count](double& pDouble, float& pFloat, int& pInt)
           //         {
           //             runTest({pDouble == 14.69, "foreach: Missmatch value"});
           //             runTest({pInt == 3, "foreach: Missmatch value"});
           //             runTest({pFloat == 2.33f, "foreach: Missmatch value"});
           //             count++;
           //         });
           //         runTest({count == 3, "foreach: Missmatch value"});
           //         count = 0;
           //     }
           // }
           // { // hasComponents tests
           //     ECS::Storage storage;
           //     { // TEST 4: HasComponents exact match multiple types.
           //         const auto entity  = storage.addEntity(1.0, 2.f, true);
           //         auto hasComponents = storage.hasComponents<double, float, bool>(entity);
           //         runTest({hasComponents == true, "hasComponents: incorrect"});
           //     }
           //     { // TEST 5: HasComponents exact match multiple types different order.
           //         const auto entity  = storage.addEntity(1.0, 2.f, true);
           //         auto hasComponents = storage.hasComponents<bool, float, double>(entity);
           //         runTest({hasComponents == true, "hasComponents: incorrect"});
           //     }
           //     { // TEST 6: HasComponents exact match single type multiple component archetype.
           //         const auto entity  = storage.addEntity(1.0, 2.f, true);
           //         auto hasComponents = storage.hasComponents<float>(entity);
           //         runTest({hasComponents == true, "hasComponents: incorrect"});
           //     }
           //     { // TEST 7: HasComponents exact match single type single component archetype.
           //         const auto entity  = storage.addEntity(1.0);
           //         auto hasComponents = storage.hasComponents<double>(entity);
           //         runTest({hasComponents == true, "hasComponents: incorrect"});
           //     }
           //     { // TEST 8: HasComponents subset match.
           //         const auto entity  = storage.addEntity(1.0, 2.f, true);
           //         auto hasComponents = storage.hasComponents<double, bool>(entity);
           //         runTest({hasComponents == true, "hasComponents: incorrect"});
           //     }
           //     { // TEST 9: HasComponents subset match different order.
           //         const auto entity  = storage.addEntity(1.0, 2.f, true);
           //         auto hasComponents = storage.hasComponents<bool, double>(entity);
           //         runTest({hasComponents == true, "hasComponents: incorrect"});
           //     }
           //     { // TEST 10: HasComponents subset match single type.
           //         const auto entity  = storage.addEntity(1.0, 2.f, true);
           //         auto hasComponents = storage.hasComponents<double>(entity);
           //         runTest({hasComponents == true, "hasComponents: incorrect"});
           //     }
           //     { // TEST 11: HasComponents no match single type.
           //         const auto entity  = storage.addEntity(1.0, 2.f, true);
           //         auto hasComponents = storage.hasComponents<std::string>(entity);
           //         runTest({hasComponents == false, "hasComponents: incorrect"});
           //     }
           //     { // TEST 12: HasComponents no match multiple types.
           //         const auto entity  = storage.addEntity(1.0, 2.f, true);
           //         auto hasComponents = storage.hasComponents<std::string, size_t>(entity);
           //         runTest({hasComponents == false, "hasComponents: incorrect"});
           //     }
           // }
           // { // foreach with Entity
           //     ECS::Storage storage;
           //     std::vector<ECS::Entity> entities;
//
           //     for (size_t i = 0; i < 12; i++)
           //         entities.push_back(storage.addEntity(1.0, 2.f, true));
//
           //     { // TEST 1: Iterate exact match archetype and count the same unique set of entities returned
           //         std::set<ECS::Entity> entitySet;
           //         storage.foreach ([&entitySet](ECS::Entity& pEntity, double& pDouble, float& pFloat, bool& pInt) { entitySet.insert(pEntity); });
           //         runTest({entitySet.size() == 12, "foreach with Entitys failed"});
           //         for (const auto& entity : entities)
           //             runTest({entitySet.contains(entity), "Entity not encountered in foreach"});
           //     }
           //     { // TEST 2: Iterate partial match archetype and count the same unique set of entities returned
           //         std::set<ECS::Entity> entitySet;
           //         storage.foreach ([&entitySet](ECS::Entity& pEntity, float& pFloat, double& pDouble) { entitySet.insert(pEntity); });
           //         runTest({entitySet.size() == 12, "foreach with Entitys failed"});
           //         for (const auto& entity : entities)
           //             runTest({entitySet.contains(entity), "Entity not encountered in foreach"});
           //     }
//
           //     // Remove all the entities in storage
           //     for (const auto& entity : entities)
           //         storage.deleteEntity(entity);
           //     entities.clear();
//
           //     { // TEST 3: foreach with no entities matching component list
           //         size_t count = 0;
           //         storage.foreach ([&count](ECS::Entity& pEntity, float& pFloat, double& pDouble) { count++; });
           //         runTest({count == 0, "There should be no entities in foreach"});
           //     }
           //     { // TEST 4: 1 entity
           //         std::set<ECS::Entity> entitySet;
           //         entities.push_back(storage.addEntity(2.0, 4.f, false)); // Add entity with different values to previously deleted data
//
           //         storage.foreach ([&entitySet](ECS::Entity& pEntity, float& pFloat, double& pDouble, bool& pBool)
           //         {
           //             entitySet.insert(pEntity);
           //             runTest({pDouble == 2.0, "foreach with Entity: missmatch value"});
           //             runTest({pFloat == 4.f, "foreach with Entity: missmatch value"});
           //             runTest({pBool == false, "foreach with Entity: missmatch value"});
           //         });
           //         runTest({entitySet.size() == 1, "There should be 1 entities in foreach"});
//
           //         for (const auto& entity : entities)
           //             runTest({entitySet.contains(entity), "Entity not encountered in foreach"});
//
           //         // Remove all the entities in storage
           //         for (const auto& entity : entities)
           //             storage.deleteEntity(entity);
           //         entities.clear();
           //     }
           //     { // TEST 4: only Entity in list
           //         std::set<ECS::Entity> entitySet;
           //         runTest({entities.size() == 0, "Test requires entities to be 0"});
//
           //         for (size_t i = 0; i < 24; i++)
           //             entities.push_back(storage.addEntity(2.0, 4.f, false)); // Add entity with different values to previously deleted data
           //         storage.foreach ([&entitySet](ECS::Entity& pEntity) { entitySet.insert(pEntity); });
//
           //         runTest({entitySet.size() == 24, "There should be 1 entities in foreach"});
           //         for (const auto& entity : entities)
           //             runTest({entitySet.contains(entity), "Entity not encountered in foreach"});
           //     }
           // }
           // LOG_INFO("ECS unit tests all passed!");
    //     }
    // }