#pragma  once

#include "int.h"


namespace Veng
{


namespace crc32_detail
{


// Generate CRC lookup table
template <unsigned c, int k = 8>
struct f : f<((c & 1) ? 0xedb88320 : 0) ^ (c >> 1), k - 1> {};
template <unsigned c> struct f<c, 0> { enum { value = c }; };

#define A(x) B(x) B(x + 128)
#define B(x) C(x) C(x +  64)
#define C(x) D(x) D(x +  32)
#define D(x) E(x) E(x +  16)
#define E(x) F(x) F(x +   8)
#define F(x) G(x) G(x +   4)
#define G(x) H(x) H(x +   2)
#define H(x) I(x) I(x +   1)
#define I(x) f<x>::value ,

constexpr unsigned crc_table[] = { A(0) };

// Constexpr implementation and helpers
constexpr u32 crc32_impl(const u8* p, size_t len, u32 crc)
{
	return len ?
		crc32_impl(p + 1, len - 1, (crc >> 8) ^ crc_table[(crc & 0xFF) ^ *p])
		: crc;
}


constexpr u32 crc32_impl_string(const u8* p, u32 crc)
{
	return *p ?
		crc32_impl_string(p + 1, (crc >> 8) ^ crc_table[(crc & 0xFF) ^ *p])
		: crc;
}


}

constexpr u32 crc32(const u8* data, size_t length)
{
	return ~crc32_detail::crc32_impl(data, length, ~0);
}

constexpr u32 crc32_string(const u8* data)
{
	return ~crc32_detail::crc32_impl_string(data, ~0);
}


#undef A
#undef B
#undef C
#undef D
#undef E
#undef F
#undef G
#undef H
#undef I


constexpr u32 Murmur3_32(const char* key, u32 seed);

u64 MurmurHash64(const void* key, u32 len, u64 seed);


}