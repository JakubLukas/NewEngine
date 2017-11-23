#pragma once

namespace Veng
{


struct Path
{
	static const unsigned MAX_LENGTH = 260;

	Path();
	Path(const char* str);
	Path(const Path& other);

	inline void operator=(const Path& other);
	inline void operator=(const char* str);

	inline bool operator==(const Path& other);
	inline bool operator!=(const Path& other);

	char path[MAX_LENGTH + 1];;
};


}