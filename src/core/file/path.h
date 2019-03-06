#pragma once

#include "core/int.h"


namespace Veng
{


class Path
{
public:
	typedef u32 Hash;
	static const size_t MAX_LENGTH = 260;
	static const size_t BUFFER_LENGTH = MAX_LENGTH + 1;

public:
	Path();
	explicit Path(const char* str);
	Path(const char* str, size_t length);
	Path(const Path& other);
	Path(const Path&& other);
	~Path() {}

	const char* GetPath() const { return m_path; }
	void SetPath(const char* str);
	Hash GetHash() const { return m_hash; }

	Path& operator=(const Path& other);
	Path& operator=(const Path&& other);

	bool operator==(const Path& other) const;
	bool operator!=(const Path& other) const;

private:
	char m_path[BUFFER_LENGTH] = { '\0' };
	Hash m_hash;
};


}