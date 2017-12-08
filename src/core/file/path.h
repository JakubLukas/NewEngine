#pragma once

namespace Veng
{


struct Path
{
	static const unsigned MAX_LENGTH = 260;

	Path();
	Path(const char* str);
	Path(const Path& other);
	Path(Path&& other);

	void Swap(Path& other);

	inline Path& operator=(const Path& other);
	inline Path& operator=(Path&& other);
	inline Path& operator=(const char* str);

	inline bool operator==(const Path& other);
	inline bool operator!=(const Path& other);

	char path[MAX_LENGTH + 1];
};


}