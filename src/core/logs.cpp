#include "logs.h"

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>


namespace Veng
{


void LogInfo(const char* format, ...)
{
	static const int BUFFER_SIZE = 4096;
	char buffer[BUFFER_SIZE];
	va_list arg;
	va_start(arg, format);
	vsnprintf(buffer, BUFFER_SIZE, format, arg);
	va_end(arg);

	OutputDebugString(buffer);
}


void LogWarning(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
}


void LogError(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}


}