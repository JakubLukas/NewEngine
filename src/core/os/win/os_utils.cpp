#include "../os_utils.h"

#define NOMINMAX
#define VC_EXTRALEAN
#include <windows.h>
#define PROCESSOR_ARCHITECTURE_ARM64 12

#include "core/asserts.h"


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


int CallProcess(const char* appPath, char* args)
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
		// Successfully created the process.  Wait for it to finish.
		ASSERT2(WAIT_OBJECT_0 == WaitForSingleObject(pi.hProcess, INFINITE), "WaitForSingleObject failed");

		// Get the exit code.
		DWORD exitCode;
		result = GetExitCodeProcess(pi.hProcess, &exitCode);

		// Close process and thread handles. 
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);

		if(result != FALSE)
		{
			return exitCode;
		}
		else
		{
			// Could not get exit code.
			ASSERT2(false, "Executed command but couldn't get exit code.");
			return 0;
		}
	}
	else
	{
		DWORD errId = ::GetLastError();
		LogErrorMessage(errId);
		ASSERT2(false, "CreateProcess failed");
		return 0;
	}
}


SystemInfo GetSystemInfo()
{
	SYSTEM_INFO sysInfo;
	::GetSystemInfo(&sysInfo);

	SystemInfo::ProcessorArchitecture procArchitecture;
	switch (sysInfo.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_INTEL:
		procArchitecture = SystemInfo::ProcessorArchitecture::Intel;
		break;
	case PROCESSOR_ARCHITECTURE_IA64:
		procArchitecture = SystemInfo::ProcessorArchitecture::IA64;
		break;
	case PROCESSOR_ARCHITECTURE_AMD64:
		procArchitecture = SystemInfo::ProcessorArchitecture::AMD64;
		break;
	case PROCESSOR_ARCHITECTURE_ARM:
		procArchitecture = SystemInfo::ProcessorArchitecture::ARM;
		break;
	case PROCESSOR_ARCHITECTURE_ARM64:
		procArchitecture = SystemInfo::ProcessorArchitecture::ARM64;
		break;
	default:
		procArchitecture = SystemInfo::ProcessorArchitecture::Unknown;
		break;
	}

	return SystemInfo
	{
		procArchitecture,
		sysInfo.dwPageSize,
		sysInfo.lpMinimumApplicationAddress,
		sysInfo.lpMaximumApplicationAddress,
		(u32)sysInfo.dwActiveProcessorMask,
		sysInfo.dwNumberOfProcessors,
		sysInfo.dwAllocationGranularity
	};
}


}


}
