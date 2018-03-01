#pragma once

#include "core/int.h"


namespace Veng
{


struct Path
{
	static const size_t MAX_LENGTH = 260;

	Path();
	Path(const char* str);
	Path(const char* str, size_t length);
	Path(const Path& other);

	Path& operator=(const Path& other);
	Path& operator=(const char* str);

	bool operator==(const Path& other);
	bool operator!=(const Path& other);

	char path[MAX_LENGTH + 1] = { '\0' };
	u32 hash;
};


}