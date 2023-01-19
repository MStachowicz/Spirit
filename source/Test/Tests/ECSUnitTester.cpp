#include "ECSUnitTester.hpp"

// TEST
#include "MemoryCorrectnessItem.hpp"

// ECS
#include "Storage.hpp"

// UTILITY
#include "Logger.hpp"

// STD
#include <set>

namespace Test
{
    size_t countEntities(ECS::Storage& pStorage)
    {
        size_t count = 0;
        pStorage.foreach([&count](const ECS::Entity& pEntity){ count++;});
        return count;
    }

    void ECSUnitTester::runAllTests()
    {
        { // ECS UNIT TESTS
            { // addEntity tests
                ECS::Storage storage;
                runTest({countEntities(storage) == 0, "Start Empty", "Storage should initialise empty"});

                int component = 42;
                storage.addEntity(component);
                runTest({countEntities(storage) == 1, "Add entity", "Storage doesnt contain 1 entity"});

                float componentFloat = 13;
                storage.addEntity(componentFloat);
                runTest({countEntities(storage) == 2, "Add second entity", "Storage doesnt contain 2 entities"});
            }
            {
                ECS::Storage storage;

                MemoryCorrectnessItem comp;
                storage.addEntity(comp);

                runTest({MemoryCorrectnessItem::countErrors() == 0, "AddEntity memory correctness", "Memory correctness error occurred adding 1 entity"});
            }

           // { // deleteEntity tests
           //     ECS::Storage storage;
           //     size_t count = 0;
//
           //     { // TEST 1: Add then delete 1 entity
           //         size_t val = 0;
           //         const auto entity = storage.addEntity(val);
           //         storage.deleteEntity(entity);
           //         storage.foreach ([&count](size_t& pComponent) { count++; });
           //         runTest({count == 0, "deleteEntity", "archetype of size_t's should be empty after 1 add and 1 delete."});
           //     }
           //     { // TEST 2: Add 3 entities - remove front back then middle.
           //         const size_t frontVal   = std::numeric_limits<size_t>::min();
           //         const auto frontEntity  = storage.addEntity(frontVal);
           //         const size_t middleVal  = 0;
           //         const auto middleEntity = storage.addEntity(middleVal);
           //         const size_t backVal    = std::numeric_limits<size_t>::max();
           //         const auto backEntity   = storage.addEntity(backVal);
//
           //         storage.deleteEntity(frontEntity);
           //         runTest({storage.getComponent<size_t>(middleEntity) == middleVal, "deleteEntity", "middle entity didn't conserve value after removing entity ahead"});
           //         runTest({storage.getComponent<size_t>(backEntity) == backVal, "deleteEntity", "back entity didn't conserve value after removing entity ahead"});
           //         count = 0;
           //         storage.foreach ([&count](size_t& pComponent) { count++; });
           //         runTest({count == 2, "deleteEntity", "archetype of size_t's should be size 2 after delete 1"});
//
           //         storage.deleteEntity(backEntity);
           //         runTest({storage.getComponent<size_t>(middleEntity) == middleVal, "deleteEntity", "middle entity didn't conserve value after removing entity ahead"});
           //         count = 0;
           //         storage.foreach ([&count](size_t& pComponent) { count++; });
           //         runTest({count == 1, "deleteEntity", "archetype of size_t's should be size 1 after 2 deletes"});
//
           //         storage.deleteEntity(middleEntity);
           //         count = 0;
           //         storage.foreach ([&count](size_t& pComponent) { count++; });
           //         runTest({count == 0, "deleteEntity", "archetype of size_t's should be size 0 after 3 deletes"});
           //     }
           //     { // TEST 3: Add delete add (overwrite previous data)
           //         size_t componentValue = 0;
           //         const auto entity0    = storage.addEntity(componentValue++);
           //         const auto entity1    = storage.addEntity(componentValue++);
           //         const auto entity2    = storage.addEntity(componentValue++);
           //         const auto entity3    = storage.addEntity(componentValue++);
//
           //         storage.deleteEntity(entity3); // Delete end entity - order unchanged
           //         count = 0;
           //         storage.foreach ([&count, this](size_t& pComponent)
           //         {
           //             runTest({pComponent == count++, "deleteEntity", "Missmatch value - Order didnt change, values in archetype should be 0-1-2"});
           //         });
           //         runTest({count == 3, "deleteEntity", "Missmatch value - There should be 3 remaining entities after 1 remove."});
//
           //         storage.deleteEntity(entity0); // Delete front entity - order changed: entity2 moved into entity0 position.
           //         // Add a new entity to the end of the archetype.
           //         // Required to overwrite the data that is still in index position 2 where entity2 used to be.
           //         // This prevents a potential false positive when testing getComponent on entity2.
           //         const auto entity4 = storage.addEntity(componentValue++);
//
           //         runTest({storage.getComponent<size_t>(entity1) == 1, "deleteEntity: Missmatch value: entity1 should have value 1 as before delete."});
           //         runTest({storage.getComponent<size_t>(entity2) == 2, "deleteEntity: Missmatch value: entity2 should have value 2 as before delete."});
           //         runTest({storage.getComponent<size_t>(entity4) == 4, "deleteEntity: Missmatch value: entity4 should have value 4."});
           //         count = 0;
           //         storage.foreach ([&count](size_t& pComponent) { count++; });
           //         runTest({count == 3, "deleteEntity: Missmatch value: Should be 3 entities after 5 addEntity and 2 deleteEntity calls"});
//
           //         // Delete the remaining entities
           //         storage.deleteEntity(entity1);
           //         storage.deleteEntity(entity2);
           //         storage.deleteEntity(entity4);
           //         count = 0;
           //         storage.foreach ([&count](size_t& pComponent) { count++; });
           //         runTest({count == 0, "deleteEntity: Missmatch value"});
           //     }
           // }
           // { // Memory correctness tests
           //     { // Test 1: Construct -> add to storage -> delete from storage -> add to storage (overwrite) -> delete again.
           //         {
           //             MemoryCorrectnessItem::reset();
           //             ECS::Storage testStorage;
//
           //             // Construct
           //             auto comp = MemoryCorrectnessItem();
           //             runTest({MemoryCorrectnessItem::countAlive() == 1, "Should be 1 component alive, is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Copy-construct - Add to storage.
           //             auto ent = testStorage.addEntity(comp);
           //             runTest({MemoryCorrectnessItem::countAlive() == 2, "Should be 2 components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Destruct - Delete from storage.
           //             testStorage.deleteEntity(ent);
           //             runTest({MemoryCorrectnessItem::countAlive() == 1, "Should be 1 component alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Copy-construct - Overwrite original in storage.
           //             ent = testStorage.addEntity(comp);
           //             runTest({MemoryCorrectnessItem::countAlive() == 2, "Should be 2 components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Destruct - Delete overwritten from storage.
           //             testStorage.deleteEntity(ent);
           //             runTest({MemoryCorrectnessItem::countAlive() == 1, "Should be 1 component alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //         } // Destruct - Comp out of scope delete.
           //         runTest({MemoryCorrectnessItem::countAlive() == 0, "Should be no components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //     }
           //     { // Test 2: Add to storage then delete front to back.
           //         {
           //             MemoryCorrectnessItem::reset();
           //             ECS::Storage testStorage;
//
           //             // Construct
           //             auto comp = MemoryCorrectnessItem();
           //             runTest({MemoryCorrectnessItem::countAlive() == 1, "Should be 1 component alive, is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Copy-construct Front and Back instances.
           //             auto entFront = testStorage.addEntity(comp);
           //             auto entBack  = testStorage.addEntity(comp);
           //             runTest({MemoryCorrectnessItem::countAlive() == 3, "Should be 3 components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Destruct - Delete Front ent in storage.
           //             testStorage.deleteEntity(entFront);
           //             runTest({MemoryCorrectnessItem::countAlive() == 2, "Should be 2 component alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
           //             // Destruct - Delete Back ent in storage.
           //             testStorage.deleteEntity(entBack);
           //             runTest({MemoryCorrectnessItem::countAlive() == 1, "Should be 1 component alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Copy-construct Front and Back instances. Overwrites the original Front and Back components in memory.
           //             entFront = testStorage.addEntity(comp);
           //             runTest({MemoryCorrectnessItem::countAlive() == 2, "Should be 2 components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
           //             entBack = testStorage.addEntity(comp);
           //             runTest({MemoryCorrectnessItem::countAlive() == 3, "Should be 3 components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Destruct - Delete Front ent in storage.
           //             testStorage.deleteEntity(entFront);
           //             runTest({MemoryCorrectnessItem::countAlive() == 2, "Should be 2 component alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
           //             // Destruct - Delete Back ent in storage.
           //             testStorage.deleteEntity(entBack);
           //             runTest({MemoryCorrectnessItem::countAlive() == 1, "Should be 1 component alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //         } // Destruct - Comp out of scope delete.
           //         runTest({MemoryCorrectnessItem::countAlive() == 0, "Should be no components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //     }
           //     { // Test 3: Add to storage then delete back to front.
           //         {
           //             MemoryCorrectnessItem::reset();
           //             ECS::Storage testStorage;
//
           //             // Construct
           //             auto comp = MemoryCorrectnessItem();
           //             runTest({MemoryCorrectnessItem::countAlive() == 1, "Should be 1 component alive, is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Copy-construct Front and Back instances.
           //             auto entFront = testStorage.addEntity(comp);
           //             auto entBack  = testStorage.addEntity(comp);
           //             runTest({MemoryCorrectnessItem::countAlive() == 3, "Should be 3 components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Destruct - Delete Back ent in storage.
           //             testStorage.deleteEntity(entBack);
           //             runTest({MemoryCorrectnessItem::countAlive() == 2, "Should be 2 component alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
           //             // Destruct - Delete Front ent in storage.
           //             testStorage.deleteEntity(entFront);
           //             runTest({MemoryCorrectnessItem::countAlive() == 1, "Should be 1 component alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Copy-construct Front and Back instances. Overwrites the original Front and Back components in memory.
           //             entFront = testStorage.addEntity(comp);
           //             runTest({MemoryCorrectnessItem::countAlive() == 2, "Should be 2 components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
           //             entBack = testStorage.addEntity(comp);
           //             runTest({MemoryCorrectnessItem::countAlive() == 3, "Should be 3 components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //             // Destruct - Delete Back ent in storage.
           //             testStorage.deleteEntity(entBack);
           //             runTest({MemoryCorrectnessItem::countAlive() == 2, "Should be 2 component alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
           //             // Destruct - Delete Front ent in storage.
           //             testStorage.deleteEntity(entFront);
           //             runTest({MemoryCorrectnessItem::countAlive() == 1, "Should be 1 component alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //             runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //         } // Destruct - Comp out of scope delete.
           //         runTest({MemoryCorrectnessItem::countAlive() == 0, "Should be no components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //     }
           //     { // Test 3: Create X Delete X Overwrite X
           //         MemoryCorrectnessItem::reset();
           //         const size_t entityCount = 100;
           //         std::vector<ECS::Entity> entities;
           //         ECS::Storage testStorage;
//
           //         // ADD
           //         for (size_t i = 0; i < entityCount; i++)
           //         {
           //             auto comp = MemoryCorrectnessItem();
           //             entities.push_back(testStorage.addEntity(comp));
           //         }
           //         runTest({MemoryCorrectnessItem::countAlive() == entityCount, "Should be {} components alive is: {}.", entityCount, MemoryCorrectnessItem::countAlive()});
           //         runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //         // DELETE
           //         for (size_t i = 0; i < entities.size(); i++)
           //             testStorage.deleteEntity(entities[i]);
           //         entities.clear();
           //         runTest({MemoryCorrectnessItem::countAlive() == 0, "Should be no components alive is: {}.", MemoryCorrectnessItem::countAlive()});
           //         runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
//
           //         // OVERWRITE
           //         for (size_t i = 0; i < entityCount; i++)
           //         {
           //             auto comp = MemoryCorrectnessItem();
           //             entities.push_back(testStorage.addEntity(comp));
           //         }
           //         runTest({MemoryCorrectnessItem::countAlive() == entityCount, "Should be {} components alive is: {}.", entityCount, MemoryCorrectnessItem::countAlive()});
           //         runTest({MemoryCorrectnessItem::countErrors() == 0, "{} memory errors occurred! read log for more info.", MemoryCorrectnessItem::countErrors()});
           //     }
           //     { // Test 4: Create X Delete in random order.
           //     }
           // }
           // { // getComponentMutable tests
            }
           // { // getComponent tests
           //     ECS::Storage storage;
           //     { // TEST 1: get front middle and end component.
           //         const auto entity = storage.addEntity(1.0, 2.f, true);
           //         runTest({storage.getComponent<double&>(entity) == 1.0, "getComponent: incorrect"});
           //         runTest({storage.getComponent<float&>(entity) == 2.0f, "getComponent: incorrect"});
           //         runTest({storage.getComponent<bool&>(entity) == true, "getComponent: incorrect"});
           //     }
           //     { // TEST 4: get single-component-archetype component.
           //         const auto entity = storage.addEntity(69.f);
           //         runTest({storage.getComponent<float&>(entity) == 69.f, "getComponent: incorrect"});
           //     }
           //     { // TEST 1: get front and back from a new archetype.
           //         const auto entity = storage.addEntity(69.69, 1.33f);
           //         runTest({storage.getComponent<double&>(entity) == 69.69, "getComponent: Missmatch value"});
           //         runTest({storage.getComponent<float&>(entity) == 1.33f, "getComponent: Missmatch value"});
           //     }
           //     { // TEST 2: Add another entity to same archetype and get.
           //         const auto entity = storage.addEntity(1.0, 4.5f);
           //         runTest({storage.getComponent<double&>(entity) == 1.0, "getComponent: Missmatch value"});
           //         runTest({storage.getComponent<float&>(entity) == 4.5f, "getComponent: Missmatch value"});
           //     }
           //     { // TEST 3: Add another entity to same archetype in different order and get.
           //         const auto entity = storage.addEntity(4.2f, 42.0);
           //         runTest({storage.getComponent<double&>(entity) == 42.0, "getComponent: Missmatch value"});
           //         runTest({storage.getComponent<float&>(entity) == 4.2f, "getComponent: Missmatch value"});
           //     }
           //     { // TEST 3: Add another entity to new archetype and get.
           //         const auto entity = storage.addEntity(true);
           //         runTest({storage.getComponent<bool&>(entity) == true, "getComponent: Missmatch value"});
           //     }
           //     { // TEST 3: Add another entity to new archetype and get.
           //         size_t val        = 69;
           //         const auto entity = storage.addEntity(val);
           //         runTest({storage.getComponent<size_t&>(entity) == 69, "getComponent: Missmatch value"});
           //     }
           // }
           // { // getComponentMutable tests
           //     {
           //      // TEST 1: edit component value
           //      // const auto entity2   = storage.addEntity(69.f);
           //      // auto& floatComponent = storage.getComponent<float&>(entity2);
           //      // floatComponent -= 69.f;
           //      // auto& floatComponentAgain = storage.getComponent<float&>(entity2);
           //      // runTest({floatComponentAgain == 0.f, "getComponent: incorrect"});
           //     }
           // }
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
        }
    }
} // namespace Test