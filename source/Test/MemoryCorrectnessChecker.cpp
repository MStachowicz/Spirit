#include "MemoryCorrectnessChecker.hpp"

namespace Test
{
    uint64_t MemoryCorrectnessItem::count_constructed      = 0;
    uint64_t MemoryCorrectnessItem::count_constructed_copy = 0;
    uint64_t MemoryCorrectnessItem::count_constructed_move = 0;
    uint64_t MemoryCorrectnessItem::count_assigned_copy    = 0;
    uint64_t MemoryCorrectnessItem::count_assigned_move    = 0;
    uint64_t MemoryCorrectnessItem::count_destroyed        = 0;

    uint64_t MemoryCorrectnessItem::errors_occurred = 0;
} // namespace Test