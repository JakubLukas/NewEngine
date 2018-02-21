#pragma once


#ifdef _WIN32
#	define FORCE_ALIGNMENT(x) __declspec(align(x))
#else
#	error Platform not supported
#endif