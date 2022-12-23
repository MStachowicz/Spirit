#include "ECSUnitTester.hpp"

// TEST
#include "MemoryCorrectnessChecker.hpp"

// ECS
#include "Storage.hpp"

// UTILITY
#include "Logger.hpp"

// STD
#include <set>

namespace Test
{
    void ECSUnitTester::runTest()
    {
        // Disabled as broken by EntityID foreach, size_t is treated as though its an EntityID.
        // Need to make ECS::EntityID a class instead of typedef
        const bool runDeleteTests = false;

        { // ECS UNIT TESTS
            {} // getComponentMutable tests
            { // getComponent tests
                ECS::Storage storage;
                { // TEST 1: get front middle and end component.
                    const auto entity = storage.addEntity(1.0, 2.f, true);
                    ZEPHYR_ASSERT(storage.getComponent<double&>(entity) == 1.0, "getComponent: incorrect");
                    ZEPHYR_ASSERT(storage.getComponent<float&>(entity) == 2.0f, "getComponent: incorrect");
                    ZEPHYR_ASSERT(storage.getComponent<bool&>(entity) == true, "getComponent: incorrect");
                }
                { // TEST 4: get single-component-archetype component.
                    const auto entity = storage.addEntity(69.f);
                    ZEPHYR_ASSERT(storage.getComponent<float&>(entity) == 69.f, "getComponent: incorrect");
                }
                { // TEST 1: get front and back from a new archetype.
                    const auto entity = storage.addEntity(69.69, 1.33f);
                    ZEPHYR_ASSERT(storage.getComponent<double&>(entity) == 69.69, "getComponent: Missmatch value");
                    ZEPHYR_ASSERT(storage.getComponent<float&>(entity) == 1.33f, "getComponent: Missmatch value");
                }
                { // TEST 2: Add another entity to same archetype and get.
                    const auto entity = storage.addEntity(1.0, 4.5f);
                    ZEPHYR_ASSERT(storage.getComponent<double&>(entity) == 1.0, "getComponent: Missmatch value");
                    ZEPHYR_ASSERT(storage.getComponent<float&>(entity) == 4.5f, "getComponent: Missmatch value");
                }
                { // TEST 3: Add another entity to same archetype in different order and get.
                    const auto entity = storage.addEntity(4.2f, 42.0);
                    ZEPHYR_ASSERT(storage.getComponent<double&>(entity) == 42.0, "getComponent: Missmatch value");
                    ZEPHYR_ASSERT(storage.getComponent<float&>(entity) == 4.2f, "getComponent: Missmatch value");
                }
                { // TEST 3: Add another entity to new archetype and get.
                    const auto entity = storage.addEntity(true);
                    ZEPHYR_ASSERT(storage.getComponent<bool&>(entity) == true, "getComponent: Missmatch value");
                }
                { // TEST 3: Add another entity to new archetype and get.
                    size_t val        = 69;
                    const auto entity = storage.addEntity(val);
                    ZEPHYR_ASSERT(storage.getComponent<size_t&>(entity) == 69, "getComponent: Missmatch value");
                }
            }
            { // getComponentMutable tests
                { // TEST 1: edit component value
                  // const auto entity2   = storage.addEntity(69.f);
                  // auto& floatComponent = storage.getComponent<float&>(entity2);
                  // floatComponent -= 69.f;
                  // auto& floatComponentAgain = storage.getComponent<float&>(entity2);
                  // ZEPHYR_ASSERT(floatComponentAgain == 0.f, "getComponent: incorrect");
                }
            }
            if (runDeleteTests)
            { // deleteEntity tests
                ECS::Storage storage;
                size_t count = 0;

                { // TEST 1: Add then delete 1 entity
                    size_t val        = 0;
                    const auto entity = storage.addEntity(val);
                    storage.deleteEntity(entity);
                    storage.foreach ([&count](size_t& pComponent)
                                     { count++; });
                    ZEPHYR_ASSERT(count == 0, "deleteEntity: archetype of size_t's should be empty after 1 add and 1 delete.");
                }
                { // TEST 2: Add 3 entities - remove front back then middle.
                    const size_t frontVal   = std::numeric_limits<size_t>::min();
                    const auto frontEntity  = storage.addEntity(frontVal);
                    const size_t middleVal  = 0;
                    const auto middleEntity = storage.addEntity(middleVal);
                    const size_t backVal    = std::numeric_limits<size_t>::max();
                    const auto backEntity   = storage.addEntity(backVal);

                    storage.deleteEntity(frontEntity);
                    ZEPHYR_ASSERT(storage.getComponent<size_t>(middleEntity) == middleVal, "deleteEntity: middle entity didn't conserve value after removing entity ahead");
                    ZEPHYR_ASSERT(storage.getComponent<size_t>(backEntity) == backVal, "deleteEntity: back entity didn't conserve value after removing entity ahead");
                    count = 0;
                    storage.foreach ([&count](size_t& pComponent)
                                     { count++; });
                    ZEPHYR_ASSERT(count == 2, "deleteEntity: archetype of size_t's should be size 2 after delete 1");

                    storage.deleteEntity(backEntity);
                    ZEPHYR_ASSERT(storage.getComponent<size_t>(middleEntity) == middleVal, "deleteEntity: middle entity didn't conserve value after removing entity ahead");
                    count = 0;
                    storage.foreach ([&count](size_t& pComponent)
                                     { count++; });
                    ZEPHYR_ASSERT(count == 1, "deleteEntity: archetype of size_t's should be size 1 after 2 deletes");

                    storage.deleteEntity(middleEntity);
                    count = 0;
                    storage.foreach ([&count](size_t& pComponent)
                                     { count++; });
                    ZEPHYR_ASSERT(count == 0, "deleteEntity: archetype of size_t's should be size 0 after 3 deletes");
                }
                { // TEST 3: Add delete add (overwrite previous data)
                    size_t componentValue = 0;
                    const auto entity0    = storage.addEntity(componentValue++);
                    const auto entity1    = storage.addEntity(componentValue++);
                    const auto entity2    = storage.addEntity(componentValue++);
                    const auto entity3    = storage.addEntity(componentValue++);

                    storage.deleteEntity(entity3); // Delete end entity - order unchanged
                    count = 0;
                    storage.foreach ([&count](size_t& pComponent)
                                     { ZEPHYR_ASSERT(pComponent == count++, "deleteEntity: Missmatch value - Order didnt change, values in archetype should be 0-1-2"); });
                    ZEPHYR_ASSERT(count == 3, "deleteEntity: Missmatch value - There should be 3 remaining entities after 1 remove.");

                    storage.deleteEntity(entity0); // Delete front entity - order changed: entity2 moved into entity0 position.
                    // Add a new entity to the end of the archetype.
                    // Required to overwrite the data that is still in index position 2 where entity2 used to be.
                    // This prevents a potential false positive when testing getComponent on entity2.
                    const auto entity4 = storage.addEntity(componentValue++);

                    ZEPHYR_ASSERT(storage.getComponent<size_t>(entity1) == 1, "deleteEntity: Missmatch value: entity1 should have value 1 as before delete.");
                    ZEPHYR_ASSERT(storage.getComponent<size_t>(entity2) == 2, "deleteEntity: Missmatch value: entity2 should have value 2 as before delete.");
                    ZEPHYR_ASSERT(storage.getComponent<size_t>(entity4) == 4, "deleteEntity: Missmatch value: entity4 should have value 4.");
                    count = 0;
                    storage.foreach ([&count](size_t& pComponent)
                                     { count++; });
                    ZEPHYR_ASSERT(count == 3, "deleteEntity: Missmatch value: Should be 3 entities after 5 addEntity and 2 deleteEntity calls");

                    // Delete the remaining entities
                    storage.deleteEntity(entity1);
                    storage.deleteEntity(entity2);
                    storage.deleteEntity(entity4);
                    count = 0;
                    storage.foreach ([&count](size_t& pComponent)
                                     { count++; });
                    ZEPHYR_ASSERT(count == 0, "deleteEntity: Missmatch value");
                }
            }
            { // forEach tests
                ECS::Storage storage;
                size_t count = 0;

                const auto entity  = storage.addEntity(13.69, 1.33f, 2);
                const auto entity2 = storage.addEntity(13.69, 1.33f, 2);
                const auto entity3 = storage.addEntity(13.69, 1.33f, 2);
                { // TEST 1: Exact match - same order as archetype.
                    storage.foreach ([&count](double& pDouble, float& pFloat, int& pInt)
                                     {
                    ZEPHYR_ASSERT(pDouble == 13.69, "foreach: Missmatch value");
                    ZEPHYR_ASSERT(pInt == 2,        "foreach: Missmatch value");
                    ZEPHYR_ASSERT(pFloat == 1.33f,  "foreach: Missmatch value");
                    count++; });
                    ZEPHYR_ASSERT(count == 3, "foreach: Missmatch value");
                    count = 0;
                }
                { // TEST 2: Exact match - different order to archetype.
                    storage.foreach ([&count](float& pFloat, int& pInt, double& pDouble)
                                     {
                    ZEPHYR_ASSERT(pDouble == 13.69, "foreach: Missmatch value");
                    ZEPHYR_ASSERT(pInt == 2,        "foreach: Missmatch value");
                    ZEPHYR_ASSERT(pFloat == 1.33f,  "foreach: Missmatch value");
                    count++; });
                    ZEPHYR_ASSERT(count == 3, "foreach: Missmatch value");
                    count = 0;
                }
                { // TEST 3: Subset match - same order as archetype.
                    storage.foreach ([&count](double& pDouble, float& pFloat)
                                     {
                    ZEPHYR_ASSERT(pDouble == 13.69, "foreach: Missmatch value");
                    ZEPHYR_ASSERT(pFloat == 1.33f,  "foreach: Missmatch value");
                    count++; });
                    ZEPHYR_ASSERT(count == 3, "foreach: Missmatch value");
                    count = 0;
                }
                { // TEST 4: Subset match - different order to archetype.
                    storage.foreach ([&count](int& pInt, float& pFloat)
                                     {
                    ZEPHYR_ASSERT(pInt == 2,        "foreach: Missmatch value");
                    ZEPHYR_ASSERT(pFloat == 1.33f,  "foreach: Missmatch value");
                    count++; });
                    ZEPHYR_ASSERT(count == 3, "foreach: Missmatch value");
                    count = 0;
                }
                { // TEST 4: Subset match single type - front of archetype.
                    storage.foreach ([&count](double& pDouble)
                                     {
                    ZEPHYR_ASSERT(pDouble == 13.69, "foreach: Missmatch value");
                    count++; });
                    ZEPHYR_ASSERT(count == 3, "foreach: Missmatch value");
                    count = 0;
                }
                { // TEST 4: Subset match single type - back of archetype.
                    storage.foreach ([&count](int& pInt)
                                     {
                    ZEPHYR_ASSERT(pInt == 2,        "foreach: Missmatch value");
                    count++; });
                    ZEPHYR_ASSERT(count == 3, "foreach: Missmatch value");
                    count = 0;
                }
                { // TEST 4: Subset match single type - middle of archetype.
                    storage.foreach ([&count](float& pFloat)
                                     {
                    ZEPHYR_ASSERT(pFloat == 1.33f,  "foreach: Missmatch value");
                    count++; });
                    ZEPHYR_ASSERT(count == 3, "foreach: Missmatch value");
                    count = 0;
                }
                { // TEST 1: Exact match change ECS data.
                    storage.foreach ([&count](double& pDouble, float& pFloat, int& pInt)
                                     {
                    pDouble += 1.0;
                    pInt += 1;
                    pFloat += 1.0f;
                    count++; });
                    ZEPHYR_ASSERT(count == 3, "foreach: Missmatch value");
                    count = 0;

                    storage.foreach ([&count](double& pDouble, float& pFloat, int& pInt)
                                     {
                    ZEPHYR_ASSERT(pDouble == 14.69, "foreach: Missmatch value");
                    ZEPHYR_ASSERT(pInt == 3,        "foreach: Missmatch value");
                    ZEPHYR_ASSERT(pFloat == 2.33f,  "foreach: Missmatch value");
                    count++; });
                    ZEPHYR_ASSERT(count == 3, "foreach: Missmatch value");
                    count = 0;
                }
            }
            { // hasComponents tests
                ECS::Storage storage;
                { // TEST 4: HasComponents exact match multiple types.
                    const auto entity  = storage.addEntity(1.0, 2.f, true);
                    auto hasComponents = storage.hasComponents<double, float, bool>(entity);
                    ZEPHYR_ASSERT(hasComponents == true, "hasComponents: incorrect");
                }
                { // TEST 5: HasComponents exact match multiple types different order.
                    const auto entity  = storage.addEntity(1.0, 2.f, true);
                    auto hasComponents = storage.hasComponents<bool, float, double>(entity);
                    ZEPHYR_ASSERT(hasComponents == true, "hasComponents: incorrect");
                }
                { // TEST 6: HasComponents exact match single type multiple component archetype.
                    const auto entity  = storage.addEntity(1.0, 2.f, true);
                    auto hasComponents = storage.hasComponents<float>(entity);
                    ZEPHYR_ASSERT(hasComponents == true, "hasComponents: incorrect");
                }
                { // TEST 7: HasComponents exact match single type single component archetype.
                    const auto entity  = storage.addEntity(1.0);
                    auto hasComponents = storage.hasComponents<double>(entity);
                    ZEPHYR_ASSERT(hasComponents == true, "hasComponents: incorrect");
                }
                { // TEST 8: HasComponents subset match.
                    const auto entity  = storage.addEntity(1.0, 2.f, true);
                    auto hasComponents = storage.hasComponents<double, bool>(entity);
                    ZEPHYR_ASSERT(hasComponents == true, "hasComponents: incorrect");
                }
                { // TEST 9: HasComponents subset match different order.
                    const auto entity  = storage.addEntity(1.0, 2.f, true);
                    auto hasComponents = storage.hasComponents<bool, double>(entity);
                    ZEPHYR_ASSERT(hasComponents == true, "hasComponents: incorrect");
                }
                { // TEST 10: HasComponents subset match single type.
                    const auto entity  = storage.addEntity(1.0, 2.f, true);
                    auto hasComponents = storage.hasComponents<double>(entity);
                    ZEPHYR_ASSERT(hasComponents == true, "hasComponents: incorrect");
                }
                { // TEST 11: HasComponents no match single type.
                    const auto entity  = storage.addEntity(1.0, 2.f, true);
                    auto hasComponents = storage.hasComponents<std::string>(entity);
                    ZEPHYR_ASSERT(hasComponents == false, "hasComponents: incorrect");
                }
                { // TEST 12: HasComponents no match multiple types.
                    const auto entity  = storage.addEntity(1.0, 2.f, true);
                    auto hasComponents = storage.hasComponents<std::string, size_t>(entity);
                    ZEPHYR_ASSERT(hasComponents == false, "hasComponents: incorrect");
                }
            }
            { // foreach with EntityIDs
                ECS::Storage storage;
                std::vector<ECS::EntityID> entities;

                for (size_t i = 0; i < 12; i++)
                    entities.push_back(storage.addEntity(1.0, 2.f, true));

                { // TEST 1: Iterate exact match archetype and count the same unique set of entities returned
                    std::set<ECS::EntityID> entitySet;
                    storage.foreach ([&entitySet](ECS::EntityID& pEntity, double& pDouble, float& pFloat, bool& pInt)
                                     { entitySet.insert(pEntity); });
                    ZEPHYR_ASSERT(entitySet.size() == 12, "foreach with entityIDs failed");
                    for (const auto& entity : entities)
                        ZEPHYR_ASSERT(entitySet.contains(entity), "Entity not encountered in foreach");
                }
                { // TEST 2: Iterate partial match archetype and count the same unique set of entities returned
                    std::set<ECS::EntityID> entitySet;
                    storage.foreach ([&entitySet](ECS::EntityID& pEntity, float& pFloat, double& pDouble)
                                     { entitySet.insert(pEntity); });
                    ZEPHYR_ASSERT(entitySet.size() == 12, "foreach with entityIDs failed");
                    for (const auto& entity : entities)
                        ZEPHYR_ASSERT(entitySet.contains(entity), "Entity not encountered in foreach");
                }

                // Remove all the entities in storage
                for (const auto& entity : entities)
                    storage.deleteEntity(entity);
                entities.clear();

                { // TEST 3: foreach with no entities matching component list
                    size_t count = 0;
                    storage.foreach ([&count](ECS::EntityID& pEntity, float& pFloat, double& pDouble)
                                     { count++; });
                    ZEPHYR_ASSERT(count == 0, "There should be no entities in foreach");
                }
                { // TEST 4: 1 entity
                    std::set<ECS::EntityID> entitySet;
                    entities.push_back(storage.addEntity(2.0, 4.f, false)); // Add entity with different values to previously deleted data

                    storage.foreach ([&entitySet](ECS::EntityID& pEntity, float& pFloat, double& pDouble, bool& pBool)
                                     {
                                     entitySet.insert(pEntity);
                                     ZEPHYR_ASSERT(pDouble == 2.0, "foreach with EntityID: missmatch value");
                                     ZEPHYR_ASSERT(pFloat == 4.f, "foreach with EntityID: missmatch value");
                                     ZEPHYR_ASSERT(pBool == false, "foreach with EntityID: missmatch value"); });
                    ZEPHYR_ASSERT(entitySet.size() == 1, "There should be 1 entities in foreach");
                    for (const auto& entity : entities)
                        ZEPHYR_ASSERT(entitySet.contains(entity), "Entity not encountered in foreach");

                    // Remove all the entities in storage
                    for (const auto& entity : entities)
                        storage.deleteEntity(entity);
                    entities.clear();
                }
                { // TEST 4: only EntityID in list
                    std::set<ECS::EntityID> entitySet;
                    ZEPHYR_ASSERT(entities.size() == 0, "Test requires entities to be 0");

                    for (size_t i = 0; i < 24; i++)
                        entities.push_back(storage.addEntity(2.0, 4.f, false)); // Add entity with different values to previously deleted data
                    storage.foreach ([&entitySet](ECS::EntityID& pEntity)
                                     { entitySet.insert(pEntity); });

                    ZEPHYR_ASSERT(entitySet.size() == 24, "There should be 1 entities in foreach");
                    for (const auto& entity : entities)
                        ZEPHYR_ASSERT(entitySet.contains(entity), "Entity not encountered in foreach");
                }
            }
            {     // Memory correctness tests
                { // Test 1: Construct -> add to storage -> delete from storage -> add to storage (overwrite) -> delete again.
                    {
                        Test::MemoryCorrectnessItem::reset();
                        ECS::Storage testStorage;

                        // Construct
                        auto comp = Test::MemoryCorrectnessItem();
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 1, "Should be 1 component alive, is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Copy-construct - Add to storage.
                        auto ent = testStorage.addEntity(comp);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 2, "Should be 2 components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Destruct - Delete from storage.
                        testStorage.deleteEntity(ent);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 1, "Should be 1 component alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Copy-construct - Overwrite original in storage.
                        ent = testStorage.addEntity(comp);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 2, "Should be 2 components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Destruct - Delete overwritten from storage.
                        testStorage.deleteEntity(ent);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 1, "Should be 1 component alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                    } // Destruct - Comp out of scope delete.
                    ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 0, "Should be no components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                }
                { // Test 2: Add to storage then delete front to back.
                    {
                        Test::MemoryCorrectnessItem::reset();
                        ECS::Storage testStorage;

                        // Construct
                        auto comp = Test::MemoryCorrectnessItem();
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 1, "Should be 1 component alive, is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Copy-construct Front and Back instances.
                        auto entFront = testStorage.addEntity(comp);
                        auto entBack  = testStorage.addEntity(comp);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 3, "Should be 3 components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Destruct - Delete Front ent in storage.
                        testStorage.deleteEntity(entFront);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 2, "Should be 2 component alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);
                        // Destruct - Delete Back ent in storage.
                        testStorage.deleteEntity(entBack);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 1, "Should be 1 component alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Copy-construct Front and Back instances. Overwrites the original Front and Back components in memory.
                        entFront = testStorage.addEntity(comp);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 2, "Should be 2 components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);
                        entBack = testStorage.addEntity(comp);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 3, "Should be 3 components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Destruct - Delete Front ent in storage.
                        testStorage.deleteEntity(entFront);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 2, "Should be 2 component alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);
                        // Destruct - Delete Back ent in storage.
                        testStorage.deleteEntity(entBack);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 1, "Should be 1 component alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                    } // Destruct - Comp out of scope delete.
                    ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 0, "Should be no components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                }
                { // Test 3: Add to storage then delete back to front.
                    {
                        Test::MemoryCorrectnessItem::reset();
                        ECS::Storage testStorage;

                        // Construct
                        auto comp = Test::MemoryCorrectnessItem();
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 1, "Should be 1 component alive, is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Copy-construct Front and Back instances.
                        auto entFront = testStorage.addEntity(comp);
                        auto entBack  = testStorage.addEntity(comp);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 3, "Should be 3 components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Destruct - Delete Back ent in storage.
                        testStorage.deleteEntity(entBack);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 2, "Should be 2 component alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);
                        // Destruct - Delete Front ent in storage.
                        testStorage.deleteEntity(entFront);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 1, "Should be 1 component alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Copy-construct Front and Back instances. Overwrites the original Front and Back components in memory.
                        entFront = testStorage.addEntity(comp);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 2, "Should be 2 components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);
                        entBack = testStorage.addEntity(comp);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 3, "Should be 3 components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                        // Destruct - Delete Back ent in storage.
                        testStorage.deleteEntity(entBack);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 2, "Should be 2 component alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);
                        // Destruct - Delete Front ent in storage.
                        testStorage.deleteEntity(entFront);
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 1, "Should be 1 component alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                        ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                    } // Destruct - Comp out of scope delete.
                    ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 0, "Should be no components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                }
                { // Test 3: Create X Delete X Overwrite X
                    Test::MemoryCorrectnessItem::reset();
                    const size_t entityCount = 100;
                    std::vector<ECS::EntityID> entities;
                    ECS::Storage testStorage;

                    // ADD
                    for (size_t i = 0; i < entityCount; i++)
                    {
                        auto comp = Test::MemoryCorrectnessItem();
                        entities.push_back(testStorage.addEntity(comp));
                    }
                    ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == entityCount, "Should be {} components alive is: {}.", entityCount, Test::MemoryCorrectnessItem::count_alive());
                    ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                    // DELETE
                    for (size_t i = 0; i < entities.size(); i++)
                        testStorage.deleteEntity(entities[i]);
                    entities.clear();
                    ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == 0, "Should be no components alive is: {}.", Test::MemoryCorrectnessItem::count_alive());
                    ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);

                    // OVERWRITE
                    for (size_t i = 0; i < entityCount; i++)
                    {
                        auto comp = Test::MemoryCorrectnessItem();
                        entities.push_back(testStorage.addEntity(comp));
                    }
                    ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::count_alive() == entityCount, "Should be {} components alive is: {}.", entityCount, Test::MemoryCorrectnessItem::count_alive());
                    ZEPHYR_ASSERT(Test::MemoryCorrectnessItem::errors_occurred == 0, "{} memory errors occurred! read log for more info.", Test::MemoryCorrectnessItem::errors_occurred);
                }
                { // Test 4: Create X Delete in random order.
                }
            }
            LOG_INFO("ECS unit tests all passed!");
        }
    }
} // namespace Test