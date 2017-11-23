#include "path.h"

#include "core/asserts.h"
#include "core/string.h"


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

Path::Path(const Path& other)
{
	operator=(other);
}


void Path::operator=(const Path& other)
{
	string::Copy(path, other.path, MAX_LENGTH);
}

void Path::operator=(const char* str)
{
	size_t len = string::Length(str);
	ASSERT(len <= MAX_LENGTH);
	string::Copy(path, str, len);
}


bool Path::operator==(const Path& other)
{
	return string::Equal(path, other.path);
}

bool Path::operator!=(const Path& other)
{
	return !(operator!=(other));
}


}