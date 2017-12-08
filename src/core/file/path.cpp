#include "path.h"

#include "core/asserts.h"
#include "core/string.h"
#include "core/utility.h"


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
	string::Copy(path, other.path, MAX_LENGTH + 1);
}

Path::Path(Path&& other)
{
	Swap(Utils::Move(other));
}


void Path::Swap(Path& other)
{
	char tmp[MAX_LENGTH + 1];
	string::Copy(tmp, other.path, MAX_LENGTH + 1);
	string::Copy(other.path, path, MAX_LENGTH + 1);
	string::Copy(path, tmp, MAX_LENGTH + 1);
}


Path& Path::operator=(const Path& other)
{
	string::Copy(path, other.path, MAX_LENGTH + 1);
	return *this;
}

Path& Path::operator=(Path&& other)
{
	Swap(Utils::Move(other));
	return *this;
}

Path& Path::operator=(const char* str)
{
	size_t len = string::Length(str);
	ASSERT(len <= MAX_LENGTH);
	string::Copy(path, str, len);
	path[len] = '\0';
	return *this;
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