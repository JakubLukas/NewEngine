#include "logs.h"

#include <cstdio>
#include <cstdarg>
#include "os/os_utils.h"


namespace Veng
{


static const int BUFFER_SIZE = 4096;
static char buffer[BUFFER_SIZE];


void Log(LogType type, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, BUFFER_SIZE, format, args);
	va_end(args);

	os::LogDebugString(buffer);
}


void Log(LogType type, const char* format, char* args)
{
	vsnprintf(buffer, BUFFER_SIZE, format, args);

	os::LogDebugString(buffer);
}


}