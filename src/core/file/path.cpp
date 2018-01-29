#include "path.h"

#include "core/asserts.h"
#include "core/string.h"
#include "core/utility.h"
#include "core/hashes.h"


namespace Veng
{


Path::Path()
{
	path[0] = '\0';
}

Path::Path(const char* str)
{
	operator=(str);
}

Path::Path(const char* str, size_t length)
{
	string::Copy(path, str, length);
	path[length] = '\0';
	hash = crc32_string((u8*)path);
}

Path::Path(const Path& other)
{
	string::Copy(path, other.path, MAX_LENGTH + 1);
	hash = crc32_string((u8*)path);
}


Path& Path::operator=(const Path& other)
{
	string::Copy(path, other.path, MAX_LENGTH + 1);
	hash = crc32_string((u8*)path);
	return *this;
}

Path& Path::operator=(const char* str)
{
	size_t len = string::Length(str);
	ASSERT(len <= MAX_LENGTH);
	string::Copy(path, str, len);
	path[len] = '\0';
	hash = crc32_string((u8*)path);
	return *this;
}


bool Path::operator==(const Path& other)
{
	return hash == other.hash;
}

bool Path::operator!=(const Path& other)
{
	return !(operator==(other));
}


}