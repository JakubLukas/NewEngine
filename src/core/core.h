#pragma once


#ifdef _WIN32
#	define FORCE_ALIGNMENT(x) __declspec(align(x))
#	define FORCE_INLINE __forceinline
#else
#	error Platform not supported
#endif
