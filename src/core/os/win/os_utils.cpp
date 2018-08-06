#include "../os_utils.h"

#include "core/asserts.h"
#define VC_EXTRALEAN
#include <windows.h>


namespace Veng
{


namespace os
{


void GetCurrentDir(char* path, size_t maxLen)
{
	GetCurrentDirectory((DWORD)maxLen, path);

	char* str = path;
	while (str[0] != '\0')
	{
		if (str[0] == '\\')
			str[0] = '/';
		str++;
	}
}


static void LogErrorMessage(DWORD errId)
{
	static const size_t MAX_MESSAGE_LENGTH = 1024;
	CHAR buffer[MAX_MESSAGE_LENGTH];

	DWORD len = ::FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errId,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buffer,
		MAX_MESSAGE_LENGTH,
		NULL
	);

	LogDebugString(buffer);
}


void LogDebugString(const char* str)
{
	::OutputDebugString(str);
}


void CallProcess(const char* appPath, char* args)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	// start the program up
	BOOL result = ::CreateProcess(appPath,   // the path
		args,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,              //creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	);

	if (result != FALSE)
	{
		// Close process and thread handles. 
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
	}
	else
	{
		DWORD errId = ::GetLastError();
		LogErrorMessage(errId);
		ASSERT(false);
	}
}


}


}