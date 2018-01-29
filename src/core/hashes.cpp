#include "hashes.h"


namespace Veng
{


/*namespace crc32_detail
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
constexpr u32 crc32_impl(const u8* p, size_t len, u32 crc) {
	return len ?
		crc32_impl(p + 1, len - 1, (crc >> 8) ^ crc_table[(crc & 0xFF) ^ *p])
		: crc;
}


}

constexpr u32 crc32(const u8* data, size_t length) {
	return ~crc32_detail::crc32_impl(data, length, ~0);
}*/


namespace murmur_detail
{

namespace detail_s
{

struct str
{
	const char* s;
	int len;
};
constexpr str stradd(const str& a, const str& b)
{
	return{ b.s, a.len + b.len };
}
constexpr str strlen(const str p, int maxdepth)
{
	return ((*p.s == 0) | (maxdepth == 0)) ? p :
		strlen({ p.s + 1, p.len + 1 }, maxdepth - 1);
}
constexpr str strlen_bychunk(const str p, int maxdepth)
{
	return *p.s == 0 ? p :
		strlen_bychunk(stradd({ 0, p.len }, strlen({ p.s, 0 }, maxdepth)), maxdepth);
}

} //namespace detail_s

  // max recursion = 256 (strlen, especially of a long string, often happens at
  // the beginning of an algorithm, so that should be fine)
constexpr int strlen(const char* s)
{
	return detail_s::strlen_bychunk(detail_s::strlen({ s, 0 }, 256), 256).len;
}

// convert char* buffer (fragment) to u32 (little-endian)
constexpr u32 word32le(const char* s, int len)
{
	return
		(len > 0 ? static_cast<u32>(s[0]) : 0)
		+ (len > 1 ? (static_cast<u32>(s[1]) << 8) : 0)
		+ (len > 2 ? (static_cast<u32>(s[2]) << 16) : 0)
		+ (len > 3 ? (static_cast<u32>(s[3]) << 24) : 0);
}
// convert char* buffer (complete) to u32 (little-endian)
constexpr u32 word32le(const char* s)
{
	return word32le(s, 4);
}


constexpr u32 murmur3_32_k(u32 k)
{
	return (((k * 0xcc9e2d51) << 15) | ((k * 0xcc9e2d51) >> 17)) * 0x1b873593;
}

constexpr u32 murmur3_32_hashround(u32 k, u32 hash)
{
	return (((hash^k) << 13) | ((hash^k) >> 19)) * 5 + 0xe6546b64;
}

constexpr u32 murmur3_32_loop(const char* key, int len, u32 hash)
{
	return len == 0 ? hash :
		murmur3_32_loop(
			key + 4,
			len - 1,
			murmur3_32_hashround(
				murmur3_32_k(word32le(key)), hash));
}

constexpr u32 murmur3_32_end0(u32 k)
{
	return (((k * 0xcc9e2d51) << 15) | ((k * 0xcc9e2d51) >> 17)) * 0x1b873593;
}

constexpr u32 murmur3_32_end1(u32 k, const char* key)
{
	return murmur3_32_end0(
		k ^ static_cast<u32>(key[0]));
}

constexpr u32 murmur3_32_end2(u32 k, const char* key)
{
	return murmur3_32_end1(
		k ^ (static_cast<u32>(key[1]) << 8), key);
}
constexpr u32 murmur3_32_end3(u32 k, const char* key)
{
	return murmur3_32_end2(
		k ^ (static_cast<u32>(key[2]) << 16), key);
}

constexpr u32 murmur3_32_end(u32 hash,
	const char* key, int rem)
{
	return rem == 0 ? hash :
		hash ^ (rem == 3 ? murmur3_32_end3(0, key) :
			rem == 2 ? murmur3_32_end2(0, key) :
			murmur3_32_end1(0, key));
}

constexpr u32 murmur3_32_final1(u32 hash)
{
	return (hash ^ (hash >> 16)) * 0x85ebca6b;
}
constexpr u32 murmur3_32_final2(u32 hash)
{
	return (hash ^ (hash >> 13)) * 0xc2b2ae35;
}
constexpr u32 murmur3_32_final3(u32 hash)
{
	return (hash ^ (hash >> 16));
}

constexpr u32 murmur3_32_final(u32 hash, int len)
{
	return
		murmur3_32_final3(
			murmur3_32_final2(
				murmur3_32_final1(hash ^ static_cast<u32>(len))));
}

constexpr u32 murmur3_32_value(const char* key, int len,
	u32 seed)
{
	return murmur3_32_final(
		murmur3_32_end(
			murmur3_32_loop(key, len / 4, seed),
			key + (len / 4) * 4, len & 3),
		len);
}

}

constexpr u32 Murmur3_32(const char *key, u32 seed)
{
	return murmur_detail::murmur3_32_value(key, murmur_detail::strlen(key), seed);
}


u64 MurmurHash64(const void* key, u32 len, u64 seed)
{
	const u64 m = 0xc6a4a7935bd1e995ULL;
	const u32 r = 47;

	u64 h = seed ^ (len * m);

	const u64* data = (const u64*)key;
	const u64* end = data + (len / 8);

	while (data != end)
	{
#ifdef PLATFORM_BIG_ENDIAN
		uint64 k = *data++;
		char *p = (char *)&k;
		char c;
		c = p[0]; p[0] = p[7]; p[7] = c;
		c = p[1]; p[1] = p[6]; p[6] = c;
		c = p[2]; p[2] = p[5]; p[5] = c;
		c = p[3]; p[3] = p[4]; p[4] = c;
#else
		u64 k = *data++;
#endif

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch (len & 7)
	{
	case 7: h ^= u64(data2[6]) << 48;
	case 6: h ^= u64(data2[5]) << 40;
	case 5: h ^= u64(data2[4]) << 32;
	case 4: h ^= u64(data2[3]) << 24;
	case 3: h ^= u64(data2[2]) << 16;
	case 2: h ^= u64(data2[1]) << 8;
	case 1: h ^= u64(data2[0]);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}


}