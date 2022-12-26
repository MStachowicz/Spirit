#include "MemoryCorrectnessChecker.hpp"

#include <iostream>

namespace Test
{
    MemoryCorrectnessItem::MemoryCorrectnessItem(const int pID /*= 0*/)
        : mID(pID)
    {
        if constexpr (verbose)
            std::cout << "Constructed mID " << mID << " at " << (void*)this << std::endl;
        if (mMemoryInitializationToken == 0x2c1dd27f0d59cf3e && mStatus != MemoryStatus::Deleted)
        {
            std::cout << "ERROR! Construction in already initialized memory at " << (void*)(this) << std::endl;
            print_memory_status();
            errorCount += 1;
        }
        mStatus                    = MemoryStatus::Constructed;
        mMemoryInitializationToken = 0x2c1dd27f0d59cf3e;
        constructedCount += 1;
    }
    MemoryCorrectnessItem::~MemoryCorrectnessItem()
    {
        if constexpr (verbose)
            std::cout << "Deleting mID " << mID << " at " << (void*)(this) << std::endl;
        if (mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            std::cout << "ERROR! Use of uninitialized memory while deleting at " << (void*)(this) << std::endl;
            errorCount += 1;
        }
        if (mStatus == MemoryStatus::Deleted)
        {
            std::cout << "ERROR! Double delete detected at " << (void*)(this) << std::endl;
            errorCount += 1;
        }
        mStatus = MemoryStatus::Deleted;
        destroyCount += 1;
    }

    MemoryCorrectnessItem::MemoryCorrectnessItem(const MemoryCorrectnessItem& other)
        : mID(other.mID)
    {
        if (other.mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            std::cout << "ERROR! Use of uninitialized memory while copy constructing from " << (void*)(&other)
                      << std::endl;
            errorCount += 1;
        }
        if (other.mStatus == MemoryStatus::Deleted)
        {
            std::cout << "ERROR! Copy constructing from deleted memory at " << (void*)(&other) << std::endl;
            errorCount += 1;
        }
        if (other.mStatus == MemoryStatus::MovedFrom)
        {
            std::cout << "ERROR! Copy constructing from moved-from memory at " << (void*)(&other) << std::endl;
            errorCount += 1;
        }
        if (mMemoryInitializationToken == 0x2c1dd27f0d59cf3e && mStatus != MemoryStatus::Deleted)
        {
            std::cout << "ERROR! Copy construction in already initialized memory at " << (void*)(this) << std::endl;
            print_memory_status();
            errorCount += 1;
        }
        if constexpr (verbose)
            std::cout << "Copy constructed mID " << mID << " from " << (void*)(&other) << " at " << (void*)this << std::endl;
        mStatus                    = MemoryStatus::Constructed;
        mMemoryInitializationToken = 0x2c1dd27f0d59cf3e;
        copyConstructCount += 1;
    }

    MemoryCorrectnessItem::MemoryCorrectnessItem(MemoryCorrectnessItem&& other)
        : mID(other.mID)
    {
        if (other.mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            std::cout << "ERROR! Use of uninitialized memory while move constructing from " << (void*)(&other) << std::endl;
            errorCount += 1;
        }
        if (other.mStatus == MemoryStatus::Deleted)
        {
            std::cout << "ERROR! Move constructing from deleted memory at " << (void*)(&other) << std::endl;
            errorCount += 1;
        }
        if (other.mStatus == MemoryStatus::MovedFrom)
        {
            std::cout << "ERROR! Move constructing from moved-from memory at " << (void*)(&other) << std::endl;
            errorCount += 1;
        }
        if (mMemoryInitializationToken == 0x2c1dd27f0d59cf3e && mStatus != MemoryStatus::Deleted)
        {
            std::cout << "ERROR! Move construction in already initialized memory at " << (void*)(this) << std::endl;
            print_memory_status();
            errorCount += 1;
        }
        other.mID     = -1;
        other.mStatus = MemoryStatus::MovedFrom;
        if constexpr (verbose)
            std::cout << "Move constructed mID " << mID << " from " << (void*)(&other) << " at " << (void*)this << std::endl;
        mStatus                    = MemoryStatus::Constructed;
        mMemoryInitializationToken = 0x2c1dd27f0d59cf3e;
        moveConstructCount += 1;
    }

    MemoryCorrectnessItem& MemoryCorrectnessItem::operator=(const MemoryCorrectnessItem& other)
    {
        if (other.mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            std::cout << "ERROR! Use of uninitialized memory while copy assigning from " << (void*)(&other)
                      << std::endl;
            errorCount += 1;
        }
        if (other.mStatus == MemoryStatus::Deleted)
        {
            std::cout << "ERROR! Copy assigning from deleted memory at " << (void*)(&other) << std::endl;
            errorCount += 1;
        }
        if (other.mStatus == MemoryStatus::MovedFrom)
        {
            std::cout << "ERROR! Copy assigning from moved-from memory at " << (void*)(&other) << std::endl;
            errorCount += 1;
        }
        if (mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            std::cout << "ERROR! Use of uninitialized memory while copy assigning to " << (void*)(this) << std::endl;
            errorCount += 1;
        }
        mID = other.mID;
        if constexpr (verbose)
            std::cout << "Copy assigning mID " << mID << " from " << (void*)(&other) << " to " << (void*)this << std::endl;
        copyAssignCount += 1;
        return *this;
    }

    MemoryCorrectnessItem& MemoryCorrectnessItem::operator=(MemoryCorrectnessItem&& other)
    {
        if (other.mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            std::cout << "ERROR! Use of uninitialized memory while move assigning from " << (void*)(&other)
                      << std::endl;
            errorCount += 1;
        }
        if (other.mStatus == MemoryStatus::Deleted)
        {
            std::cout << "ERROR! Move assigning from deleted memory at " << (void*)(&other) << std::endl;
            errorCount += 1;
        }
        if (other.mStatus == MemoryStatus::MovedFrom)
        {
            std::cout << "ERROR! Move assigning from moved-from memory at " << (void*)(&other) << std::endl;
            errorCount += 1;
        }
        if (mMemoryInitializationToken != 0x2c1dd27f0d59cf3e)
        {
            std::cout << "ERROR! Use of uninitialized memory while move assigning to " << (void*)(this) << std::endl;
            errorCount += 1;
        }
        mID           = other.mID;
        other.mID     = -1;
        other.mStatus = MemoryStatus::MovedFrom;
        if constexpr (verbose)
            std::cout << "Move assigning mID " << mID << " from " << (void*)(&other) << " to " << (void*)this << std::endl;
        moveAssignCount += 1;
        return *this;
    }

    void MemoryCorrectnessItem::print_memory_status()
    {
        if ((int)mStatus >= 0 && (int)mStatus <= 3)
            std::cout << "The memory mStatus was: " << MemoryStatusNames[(int)mStatus] << std::endl;
        else
            std::cout << "The memory mStatus was: " << (int)mStatus << std::endl;
    }
} // namespace Test