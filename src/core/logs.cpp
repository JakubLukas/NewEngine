#include "logs.h"

#include <stdio.h>
#include <stdarg.h>
#define VC_EXTRALEAN
#include <windows.h>


namespace Veng
{


static const int BUFFER_SIZE = 4096;
char buffer[BUFFER_SIZE];


void LogInfo(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, BUFFER_SIZE, format, args);
	va_end(args);

	OutputDebugString(buffer);
}


void LogWarning(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, BUFFER_SIZE, format, args);
	va_end(args);

	OutputDebugString(buffer);
}


void LogError(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, BUFFER_SIZE, format, args);
	va_end(args);

	OutputDebugString(buffer);
}


}