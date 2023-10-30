#pragma once

#include <string>

namespace Test
{
    class MemoryCorrectnessItem
    {
        constexpr static bool verbose = true;

        enum class MemoryStatus
        {
            Uninitialized = 0,
            Constructed   = 1,
            MovedFrom     = 2,
            Deleted       = 3
        };

        inline static size_t constructedCount   = 0;
        inline static size_t destroyCount       = 0;
        inline static size_t copyConstructCount = 0;
        inline static size_t moveConstructCount = 0;
        inline static size_t copyAssignCount    = 0;
        inline static size_t moveAssignCount    = 0;
        inline static size_t errorCount         = 0;

        inline static size_t instanceID         = 0; // Unique ID per instance of MemoryCorrectnessItem

        size_t mID;
        volatile MemoryStatus mStatus;
        volatile size_t mMemoryInitializationToken;
        // Padding to push the memory_status back a little
        // Without this, some tests seem to generate false positive errors, which could be due to the memory at the start
        // of the object being reused for something else after deletion, changing the memory status while leaving the
        // token intact.
        char mPadding[16];

        std::string toString() const;
        std::string getMemoryStatus() const;
        std::string toStringAndMemoryStatus() const;
    public:
        static void reset();
        static size_t countAlive()
        {
            return constructedCount + copyConstructCount + moveConstructCount - destroyCount;
        }
        static size_t countErrors()
        {
            return errorCount;
        }

        MemoryCorrectnessItem();
        ~MemoryCorrectnessItem();
        MemoryCorrectnessItem(const MemoryCorrectnessItem& pOther);
        MemoryCorrectnessItem(MemoryCorrectnessItem&& pOther);
        MemoryCorrectnessItem& operator=(const MemoryCorrectnessItem& pOther);
        MemoryCorrectnessItem& operator=(MemoryCorrectnessItem&& pOther);
    };
} // namespace Test