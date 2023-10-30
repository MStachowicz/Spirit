#include "MemoryCorrectnessItem.hpp"

#include "Logger.hpp"

#include <format>
#include <iostream>

namespace Test
{
    MemoryCorrectnessItem::MemoryCorrectnessItem()
        : mID(instanceID++)
    {
        if constexpr (verbose)
            LOG("Constructing {}", toString());

        if (mMemoryInitializationToken == 0x2c1dd27f0d59cf3e && mStatus != MemoryStatus::Deleted)
        {
            LOG_ERROR("ERROR! Construction in already initialized memory at {}", toStringAndMemoryStatus());
            errorCount += 1;
        }

        mStatus                    = MemoryStatus::Constructed;
        mMemoryInitializationToken = 0x2c1dd27f0d59cf3e;
        constructedCount += 1;
    }
    MemoryCorrectnessItem::~MemoryCorrectnessItem()
    {
        if constexpr (verbose)
            LOG("Deleting {}", toString());

        if (mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            LOG_ERROR("ERROR! Use of uninitialized memory while deleting at {}", toString());
            errorCount += 1;
        }
        if (mStatus == MemoryStatus::Deleted)
        {
            LOG_ERROR("ERROR! Double delete detected at {}", toString());
            errorCount += 1;
        }

        mStatus = MemoryStatus::Deleted;
        destroyCount += 1;
    }

    MemoryCorrectnessItem::MemoryCorrectnessItem(const MemoryCorrectnessItem& pOther)
        : mID(instanceID++)
    {
        if constexpr (verbose)
            LOG("Copy constructing {} from {}", toString(), pOther.toString());

        if (pOther.mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            LOG_ERROR("ERROR! Use of uninitialized memory while copy constructing from {}", pOther.toString());
            errorCount += 1;
        }
        if (pOther.mStatus == MemoryStatus::Deleted)
        {
            LOG_ERROR("ERROR! Copy constructing from deleted memory at {}", pOther.toString());
            errorCount += 1;
        }
        if (pOther.mStatus == MemoryStatus::MovedFrom)
        {
            LOG_ERROR("ERROR! Copy constructing from moved-from memory at {}", pOther.toString());
            errorCount += 1;
        }

        if (mMemoryInitializationToken == 0x2c1dd27f0d59cf3e && mStatus != MemoryStatus::Deleted)
        {
            LOG_ERROR("ERROR! Copy construction in already initialized memory at {}", toStringAndMemoryStatus());
            errorCount += 1;
        }

        mStatus = MemoryStatus::Constructed;
        mMemoryInitializationToken = 0x2c1dd27f0d59cf3e;
        copyConstructCount += 1;
    }

    MemoryCorrectnessItem::MemoryCorrectnessItem(MemoryCorrectnessItem&& pOther)
        : mID(instanceID++)
    {
        if constexpr (verbose)
            LOG("Move constructing {} from {}", toString(), pOther.toString());

        if (pOther.mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            LOG_ERROR("ERROR! Use of uninitialized memory while move constructing from {}", pOther.toString());
            errorCount += 1;
        }
        if (pOther.mStatus == MemoryStatus::Deleted)
        {
            LOG_ERROR("ERROR! Move constructing from deleted memory at {}", pOther.toString());
            errorCount += 1;
        }
        if (pOther.mStatus == MemoryStatus::MovedFrom)
        {
            LOG_ERROR("ERROR! Move constructing from moved-from memory at {}", pOther.toString());
            errorCount += 1;
        }

        if (mMemoryInitializationToken == 0x2c1dd27f0d59cf3e && mStatus != MemoryStatus::Deleted)
        {
            LOG_ERROR("ERROR! Move construction in already initialized memory at {}", toStringAndMemoryStatus());
            errorCount += 1;
        }

        pOther.mStatus              = MemoryStatus::MovedFrom;
        mStatus                     = MemoryStatus::Constructed;
        mMemoryInitializationToken  = 0x2c1dd27f0d59cf3e;
        moveConstructCount         += 1;
    }

    MemoryCorrectnessItem& MemoryCorrectnessItem::operator=(const MemoryCorrectnessItem& pOther)
    {
        if constexpr (verbose)
            LOG("Copy assigning {} from {}", toString(), pOther.toString());

        if (pOther.mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            LOG_ERROR("ERROR! Use of uninitialized memory while copy assigning from {}", pOther.toString());
            errorCount += 1;
        }
        if (pOther.mStatus == MemoryStatus::Deleted)
        {
            LOG_ERROR("ERROR! Copy assigning from deleted memory at {}", pOther.toString());
            errorCount += 1;
        }
        if (pOther.mStatus == MemoryStatus::MovedFrom)
        {
            LOG_ERROR("ERROR! Copy assigning from moved-from memory at {}", pOther.toString());
            errorCount += 1;
        }

        if (mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            LOG_ERROR("ERROR! Use of uninitialized memory while copy assigning to {}", toString());
            errorCount += 1;
        }

        copyAssignCount += 1;
        return *this;
    }

    MemoryCorrectnessItem& MemoryCorrectnessItem::operator=(MemoryCorrectnessItem&& pOther)
    {
        if constexpr (verbose)
            LOG("Move assigning {} from {}", toString(), pOther.toString());

        if (pOther.mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            LOG_ERROR("ERROR! Use of uninitialized memory while move assigning from {}", pOther.toString());
            errorCount += 1;
        }
        if (pOther.mStatus == MemoryStatus::Deleted)
        {
            LOG_ERROR("ERROR! Move assigning from deleted memory at {}", pOther.toString());
            errorCount += 1;
        }
        if (pOther.mStatus == MemoryStatus::MovedFrom)
        {
            LOG_ERROR("ERROR! Move assigning from moved-from memory at {}", pOther.toString());
            errorCount += 1;
        }

        if (mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            LOG_ERROR("ERROR! Use of uninitialized memory while move assigning to {}", toString());
            errorCount += 1;
        }

        pOther.mStatus   = MemoryStatus::MovedFrom;
        mID              = instanceID++;
        moveAssignCount += 1;
        return *this;
    }

    std::string MemoryCorrectnessItem::getMemoryStatus() const
    {
        const static char* memoryStatusNames[4] = {"Uninitialized", "Constructed", "MovedFrom", "Deleted"};

        if ((int)mStatus >= 0 && (int)mStatus <= 3)
            return std::format("Memory status was: {}", memoryStatusNames[(int)mStatus]);
        else
            return std::format("Memory status was: {}", (int)mStatus);
    }
    std::string MemoryCorrectnessItem::toString() const
    {
        return std::format("ID: {} ({})", mID, (void*)(this));
    }
    std::string MemoryCorrectnessItem::toStringAndMemoryStatus() const
    {
        return std::format("{} - {}", toString(), getMemoryStatus());
    }

    void MemoryCorrectnessItem::reset()
    {
        constructedCount   = 0;
        destroyCount       = 0;
        copyConstructCount = 0;
        moveConstructCount = 0;
        copyAssignCount    = 0;
        moveAssignCount    = 0;
        errorCount         = 0;

        if constexpr (verbose)
            LOG("RESET MemoryCorrectnessItem");
    }
} // namespace Test