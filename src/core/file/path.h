#pragma once

#include "core/int.h"


namespace Veng
{


struct Path
{
	static const size_t MAX_LENGTH = 260;

	static u32 GetHash(const Path& path);

	Path();
	Path(const char* str);
	Path(const Path& other);
	Path(Path&& other);

	void Swap(Path& other);

	Path& operator=(const Path& other);
	Path& operator=(Path&& other);
	Path& operator=(const char* str);

	bool operator==(const Path& other);
	bool operator!=(const Path& other);

	char path[MAX_LENGTH + 1];
};


}