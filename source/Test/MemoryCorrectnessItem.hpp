#pragma once

#include <string>
#include <optional>

namespace Test
{
    class MemoryCorrectnessItem
    {
        constexpr static bool LOG_MEM_CORRECTNESS_EVENTS = false;

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

        size_t mID; // Unique ID of a constructed MemoryCorrectnessItem instance.
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
		std::optional<int> m_member; // A faux member to emulate a resource storage of the object.

        size_t ID() const { return mID; }
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