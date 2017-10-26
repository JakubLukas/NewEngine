#pragma  once

#include "int.h"


namespace Veng
{


constexpr u32 crc32(const u8* data, size_t length);

constexpr u32 Murmur3_32(const char* key, u32 seed);

u64 MurmurHash64(const void* key, u32 len, u64 seed);


}