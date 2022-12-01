#include "platform.h"

uint8 * 
GetMemoryFromArena(uint32 bytes_requested, MemoryArena * memory_arena)
{
    if (bytes_requested > memory_arena->size - memory_arena->consumed)
    {
        ASSERT(0);
    }
    uint8 * memory_start = memory_arena->first_byte + memory_arena->consumed;
    memory_arena->consumed += bytes_requested;
    return memory_start;
}

