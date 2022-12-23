#pragma once

#include <iostream>

namespace Test
{
    class MemoryCorrectnessItem
    {
    public:
        constexpr static bool verbose = false;

        MemoryCorrectnessItem(int id = 0)
            : id(id)
        {
            if constexpr (verbose)
                std::cout << "Constructed id " << id << " at " << (void*)this << std::endl;
            if (memory_initialization_token == 0x2c1dd27f0d59cf3e && status != MemoryStatus::Deleted)
            {
                std::cout << "ERROR! Construction in already initialized memory at " << (void*)(this) << std::endl;
                print_memory_status();
                errors_occurred += 1;
            }
            status                      = MemoryStatus::Constructed;
            memory_initialization_token = 0x2c1dd27f0d59cf3e;
            count_constructed += 1;
        }

        MemoryCorrectnessItem(const MemoryCorrectnessItem& other)
            : id(other.id)
        {
            if (other.memory_initialization_token != 0x2c1dd27f0d59cf3e)
            {
                std::cout << "ERROR! Use of uninitialized memory while copy constructing from " << (void*)(&other)
                          << std::endl;
                errors_occurred += 1;
            }
            if (other.status == MemoryStatus::Deleted)
            {
                std::cout << "ERROR! Copy constructing from deleted memory at " << (void*)(&other) << std::endl;
                errors_occurred += 1;
            }
            if (other.status == MemoryStatus::MovedFrom)
            {
                std::cout << "ERROR! Copy constructing from moved-from memory at " << (void*)(&other) << std::endl;
                errors_occurred += 1;
            }
            if (memory_initialization_token == 0x2c1dd27f0d59cf3e && status != MemoryStatus::Deleted)
            {
                std::cout << "ERROR! Copy construction in already initialized memory at " << (void*)(this) << std::endl;
                print_memory_status();
                errors_occurred += 1;
            }
            if constexpr (verbose)
                std::cout << "Copy constructed id " << id << " from " << (void*)(&other) << " at " << (void*)this << std::endl;
            status                      = MemoryStatus::Constructed;
            memory_initialization_token = 0x2c1dd27f0d59cf3e;
            count_constructed_copy += 1;
        }

        MemoryCorrectnessItem(MemoryCorrectnessItem&& other)
            : id(other.id)
        {
            if (other.memory_initialization_token != 0x2c1dd27f0d59cf3e)
            {
                std::cout << "ERROR! Use of uninitialized memory while move constructing from " << (void*)(&other) << std::endl;
                errors_occurred += 1;
            }
            if (other.status == MemoryStatus::Deleted)
            {
                std::cout << "ERROR! Move constructing from deleted memory at " << (void*)(&other) << std::endl;
                errors_occurred += 1;
            }
            if (other.status == MemoryStatus::MovedFrom)
            {
                std::cout << "ERROR! Move constructing from moved-from memory at " << (void*)(&other) << std::endl;
                errors_occurred += 1;
            }
            if (memory_initialization_token == 0x2c1dd27f0d59cf3e && status != MemoryStatus::Deleted)
            {
                std::cout << "ERROR! Move construction in already initialized memory at " << (void*)(this) << std::endl;
                print_memory_status();
                errors_occurred += 1;
            }
            other.id     = -1;
            other.status = MemoryStatus::MovedFrom;
            if constexpr (verbose)
                std::cout << "Move constructed id " << id << " from " << (void*)(&other) << " at " << (void*)this << std::endl;
            status                      = MemoryStatus::Constructed;
            memory_initialization_token = 0x2c1dd27f0d59cf3e;
            count_constructed_move += 1;
        }

        MemoryCorrectnessItem& operator=(const MemoryCorrectnessItem& other)
        {
            if (other.memory_initialization_token != 0x2c1dd27f0d59cf3e)
            {
                std::cout << "ERROR! Use of uninitialized memory while copy assigning from " << (void*)(&other)
                          << std::endl;
                errors_occurred += 1;
            }
            if (other.status == MemoryStatus::Deleted)
            {
                std::cout << "ERROR! Copy assigning from deleted memory at " << (void*)(&other) << std::endl;
                errors_occurred += 1;
            }
            if (other.status == MemoryStatus::MovedFrom)
            {
                std::cout << "ERROR! Copy assigning from moved-from memory at " << (void*)(&other) << std::endl;
                errors_occurred += 1;
            }
            if (memory_initialization_token != 0x2c1dd27f0d59cf3e)
            {
                std::cout << "ERROR! Use of uninitialized memory while copy assigning to " << (void*)(this) << std::endl;
                errors_occurred += 1;
            }
            id = other.id;
            if constexpr (verbose)
                std::cout << "Copy assigning id " << id << " from " << (void*)(&other) << " to " << (void*)this << std::endl;
            count_assigned_copy += 1;
            return *this;
        }

        MemoryCorrectnessItem& operator=(MemoryCorrectnessItem&& other)
        {
            if (other.memory_initialization_token != 0x2c1dd27f0d59cf3e)
            {
                std::cout << "ERROR! Use of uninitialized memory while move assigning from " << (void*)(&other)
                          << std::endl;
                errors_occurred += 1;
            }
            if (other.status == MemoryStatus::Deleted)
            {
                std::cout << "ERROR! Move assigning from deleted memory at " << (void*)(&other) << std::endl;
                errors_occurred += 1;
            }
            if (other.status == MemoryStatus::MovedFrom)
            {
                std::cout << "ERROR! Move assigning from moved-from memory at " << (void*)(&other) << std::endl;
                errors_occurred += 1;
            }
            if (memory_initialization_token != 0x2c1dd27f0d59cf3e)
            {
                std::cout << "ERROR! Use of uninitialized memory while move assigning to " << (void*)(this) << std::endl;
                errors_occurred += 1;
            }
            id           = other.id;
            other.id     = -1;
            other.status = MemoryStatus::MovedFrom;
            if constexpr (verbose)
                std::cout << "Move assigning id " << id << " from " << (void*)(&other) << " to " << (void*)this << std::endl;
            count_assigned_move += 1;
            return *this;
        }

        ~MemoryCorrectnessItem()
        {
            if constexpr (verbose)
                std::cout << "Deleting id " << id << " at " << (void*)(this) << std::endl;
            if (memory_initialization_token != 0x2c1dd27f0d59cf3e)
            {
                std::cout << "ERROR! Use of uninitialized memory while deleting at " << (void*)(this) << std::endl;
                errors_occurred += 1;
            }
            if (status == MemoryStatus::Deleted)
            {
                std::cout << "ERROR! Double delete detected at " << (void*)(this) << std::endl;
                errors_occurred += 1;
            }
            status = MemoryStatus::Deleted;
            count_destroyed += 1;
        }

        int id;

        // Padding to push the memory_status back a little
        // Without this, some tests seem to generate false positive errors, which could be due to the memory at the start
        // of the object being reused for something else after deletion, changing the memory status while leaving the
        // token intact.
        char padding[16];

        enum class MemoryStatus
        {
            Uninitialized = 0,
            Constructed   = 1,
            MovedFrom     = 2,
            Deleted       = 3
        };

        const char* MemoryStatusNames[4] = {
            "Uninitialized",
            "Constructed",
            "MovedFrom",
            "Deleted"};

        void print_memory_status()
        {
            if ((int)status >= 0 && (int)status <= 3)
                std::cout << "The memory status was: " << MemoryStatusNames[(int)status] << std::endl;
            else
                std::cout << "The memory status was: " << (int)status << std::endl;
        }

        volatile MemoryStatus status;
        volatile uint64_t memory_initialization_token;

        static uint64_t count_alive()
        {
            return count_constructed + count_constructed_copy + count_constructed_move - count_destroyed;
        }

        static void reset()
        {
            count_constructed      = 0;
            count_constructed_copy = 0;
            count_constructed_move = 0;
            count_assigned_copy    = 0;
            count_assigned_move    = 0;
            count_destroyed        = 0;

            errors_occurred = 0;
        }

        static uint64_t count_constructed;
        static uint64_t count_constructed_copy;
        static uint64_t count_constructed_move;
        static uint64_t count_assigned_copy;
        static uint64_t count_assigned_move;
        static uint64_t count_destroyed;

        static uint64_t errors_occurred;
    };
} // namespace Test