#include "path.h"

#include "core/asserts.h"
#include "core/string.h"
#include "core/utility.h"
#include "core/hashes.h"


namespace Veng
{


Path::Path()
{
	m_path[0] = '\0';
}

Path::Path(const char* str)
{
	SetPath(str);
}

Path::Path(const char* str, size_t length)
{
	string::Copy(m_path, str, length);
	m_path[length] = '\0';
	FixSlashes();
	m_hash = crc32_string(m_path);
}

Path::Path(const Path& other)
	: m_hash(other.m_hash)
{
	string::Copy(m_path, other.m_path, BUFFER_LENGTH);
}

Path::Path(const Path&& other)
	: m_hash(other.m_hash)
{
	string::Copy(m_path, other.m_path, BUFFER_LENGTH);
}


void Path::SetPath(const char* str)
{
	size_t len = string::Length(str);
	ASSERT(len <= MAX_LENGTH);
	string::Copy(m_path, str, len);
	m_path[len] = '\0';
	FixSlashes();
	m_hash = crc32_string(m_path);
}


Path& Path::operator=(const Path& other)
{
	string::Copy(m_path, other.m_path, BUFFER_LENGTH);
	m_hash = other.m_hash;
	return *this;
}

Path& Path::operator=(const Path&& other)
{
	string::Copy(m_path, other.m_path, BUFFER_LENGTH);
	m_hash = other.m_hash;
	return *this;
}


bool Path::operator==(const Path& other) const
{
	return m_hash == other.m_hash;
}

bool Path::operator!=(const Path& other) const
{
	return !(operator==(other));
}


void Path::FixSlashes()
{
	char* path = m_path;
	while (*path != '\0')
	{
		if (*path == '\\') *path = '/';
		path++;
	}
}


}