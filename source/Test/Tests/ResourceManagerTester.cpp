#include "ResourceManagerTester.hpp"
#include "MemoryCorrectnessItem.hpp"
#include "ResourceManager.hpp"

namespace Test
{
    using Ref     = Utility::ResourceRef<MemoryCorrectnessItem>;
    using Manager = Utility::ResourceManager<MemoryCorrectnessItem>;

    void Test::ResourceManagerTester::runUnitTests()
    {
        {// Check ResourceRef API
            MemoryCorrectnessItem::reset();

            auto ref = Ref();
            CHECK_EQUAL(ref.is_valid(), false, "ResourceRef isValid() check");
            CHECK_TRUE(ref ? false : true,     "ResourceRef explicit bool operator check");

            Manager manager;
            auto ref2 = manager.insert(MemoryCorrectnessItem{});
            CHECK_EQUAL(ref2->countAlive(), 1,   "Use the Resource via the Ref");
            CHECK_EQUAL((*ref2).countAlive(), 1, "Use the Resource via the Ref");

            const auto ref3 = manager.insert(MemoryCorrectnessItem{});
            CHECK_EQUAL(ref3->countAlive(), 2,   "Use the Resource via the Ref");
            CHECK_EQUAL((*ref3).countAlive(), 2, "Use the Resource via the Ref");
        }
        CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 0, "Memory leak check");
        CHECK_EQUAL(MemoryCorrectnessItem::countErrors(), 0, "Memory Error check");

        { // Check memory leaks single insert
            MemoryCorrectnessItem::reset();

            {
                Manager manager;
                {
                    auto ref = manager.insert(MemoryCorrectnessItem{});
                    CHECK_EQUAL(manager.size(), 1, "Size check after insert");
                }
                CHECK_EQUAL(manager.size(), 0, "Size check after destroyed ref");
                CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 0, "Memory leak check ref deleted");
                CHECK_EQUAL(MemoryCorrectnessItem::countErrors(), 0, "Memory Error check ref deleted");
            }
            CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 0, "Memory leak check");
            CHECK_EQUAL(MemoryCorrectnessItem::countErrors(), 0, "Memory Error check");
        }
        {// Check memory leaks many inserts
            MemoryCorrectnessItem::reset();

            Manager manager;
            manager.reserve(100);
            {
                std::vector<Ref> refs; // Maintain their lifetime in vector
                refs.reserve(100);
                for (auto i = 0; i < 100; i++)
                    refs.push_back(manager.insert(MemoryCorrectnessItem{}));
                CHECK_EQUAL(manager.size(), 100, "Size check after insert 100");
            }
            CHECK_EQUAL(manager.size(), 0, "Size check after insert 100 deleted");
            CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 0, "Memory leak check");
            CHECK_EQUAL(MemoryCorrectnessItem::countErrors(), 0, "Memory error check");
        }
        {// Check capacity change maintains resource validity
            MemoryCorrectnessItem::reset();

            {
                Manager manager;
                auto ref = manager.insert(MemoryCorrectnessItem{});
                manager.reserve(manager.capacity() * 2);
                CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 1, "Memory leak check");
            }

            CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 0, "Memory leak check");
            CHECK_EQUAL(MemoryCorrectnessItem::countErrors(), 0, "Memory Error check");
        }
        {// Check capacity change maintains resource validity many inserts
            MemoryCorrectnessItem::reset();

            {
                Manager manager;
                ASSERT(manager.capacity() < 100, "Capacity has to be below 100 for test to work.");
                std::vector<Ref> refs; // Maintain their lifetime in vector
                refs.reserve(100);

                for (auto i = 0; i < 100; i++)
                    refs.push_back(manager.insert(MemoryCorrectnessItem{}));

                CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 100, "Memory leak check");
            }

            CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 0, "Memory leak check");
            CHECK_EQUAL(MemoryCorrectnessItem::countErrors(), 0, "Memory Error check");
        }
        //{// Check memory leaks Manager::clear()
        //    MemoryCorrectnessItem::reset();
//
        //    Manager manager;
        //    {
        //        std::vector<Ref> refs; // Maintain their lifetime in vector
        //        refs.reserve(100);
        //        for (auto i = 0; i < 100; i++)
        //            refs.push_back(manager.insert(MemoryCorrectnessItem{}));
//
        //        CHECK_EQUAL(manager.size(), 100, "Size check after insert 100");
        //        manager.clear();
        //        CHECK_EQUAL(manager.size(), 0, "Size check after clear");
        //    }
        //    CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 0, "Memory leak check");
        //    CHECK_EQUAL(MemoryCorrectnessItem::countErrors(), 0, "Memory error check");
        //}
        {// Check capacity change maintains resource validity with many resources
            MemoryCorrectnessItem::reset();

            Manager manager;
            for (auto i = 0; i < 100; i++)
                auto ref = manager.insert(MemoryCorrectnessItem{});
            manager.reserve(manager.capacity() * 2);

            CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 0, "Memory leak check");
            CHECK_EQUAL(MemoryCorrectnessItem::countErrors(), 0, "Memory Error check");
        }
        {// Create an invalid resource and assign it a valid resource
            MemoryCorrectnessItem::reset();

            {
                Manager manager;

                auto ref = Ref();
                CHECK_EQUAL(manager.size(), 0, "Size check after invalid ref");

                ref = manager.insert(MemoryCorrectnessItem{});
                CHECK_TRUE(ref.is_valid(), "Invalid ResourceRef is valid after assigning");
                CHECK_EQUAL(manager.size(), 1, "Size check after assigning to an invalid ref");

                auto ref2 = manager.insert(MemoryCorrectnessItem{});
                CHECK_EQUAL(manager.size(), 2, "Size check after inserting a second resource");

                // Copying a ref should give us access to the same resource and not change the size.
                auto ref_copy = ref;
                CHECK_TRUE(ref_copy.is_valid(), "ResourceRef copy is valid");
                CHECK_EQUAL(manager.size(), 2, "Size check after copying a ResourceRef");
            }
            CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 0, "Memory leak check");
            CHECK_EQUAL(MemoryCorrectnessItem::countErrors(), 0, "Memory Error check");
        }
        {// Create an valid resource and assign it a valid resource
            MemoryCorrectnessItem::reset();
            {
                Manager manager;
                //manager.reserve(4);

                auto ref = manager.insert(MemoryCorrectnessItem{}); // Construct, Move constructing, Delete
                ref = manager.insert(MemoryCorrectnessItem{});      // Construct (new one), Move-assign, Delete
                CHECK_TRUE(ref.is_valid(), "Check ref is valid after being assigned while already owning a resource");
                CHECK_EQUAL(manager.size(), 1, "Size remains the same after assigning to a valid ref");
                CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 1, "Memory leak after move-assigning a valid ref");
            }
            CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 0, "Memory leak check");
            CHECK_EQUAL(MemoryCorrectnessItem::countErrors(), 0, "Memory Error check");
        }
        {// Check Resource data is intact after a second insert
            MemoryCorrectnessItem::reset();
            {
                Manager manager;
                auto ref_1 = manager.insert(MemoryCorrectnessItem{});
                auto ID = ref_1->ID();
                auto ref_2 = manager.insert(MemoryCorrectnessItem{});
                CHECK_EQUAL(ref_1->ID(), ID, "Check data intact after a second insert");
            }
            CHECK_EQUAL(MemoryCorrectnessItem::countAlive(), 0, "Memory leak check");
            CHECK_EQUAL(MemoryCorrectnessItem::countErrors(), 0, "Memory Error check");
        }


        {// TODO test get_or_create
        }
        {// TODO Check move assigning and move constructing a ResourceManager
        }
        {// TODO check Ref is_valid() == false after the manager is cleared?
        }
    }

    void Test::ResourceManagerTester::runPerformanceTests() {}
} // namespace Test
