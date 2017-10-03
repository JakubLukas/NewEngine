#pragma once


#ifdef _WIN32
#	ifdef _WIN64
#		define PLATFORM64
#	else
#		define PLATFORM32
#	endif
#elif defined(__linux__)
#	ifdef __x86_64__
#		define PLATFORM64
#	else
#		define PLATFORM32
#	endif
#else
#	error Platform not supported
#endif


typedef char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;

#ifdef _WIN32
	typedef long long i64;
	typedef unsigned long long u64;
#else
#	ifdef PLATFORM64
		typedef long i64;
		typedef unsigned long u64;
#	else
		typedef long long i64;
		typedef unsigned long long u64;
#	endif
#endif

#ifdef PLATFORM64
	typedef u64 uintptr;
#else
	typedef u32 uintptr;
#endif

typedef size_t size;

static_assert(sizeof(i8) == 1, "Incorrect size of i8");
static_assert(sizeof(i16) == 2, "Incorrect size of i16");
static_assert(sizeof(i32) == 4, "Incorrect size of i32");
static_assert(sizeof(i64) == 8, "Incorrect size of i64");
static_assert(sizeof(uintptr) == sizeof(void*), "Incorrect size of uintptr");
static_assert(sizeof(size) == sizeof(size_t), "Incorrect size of size");