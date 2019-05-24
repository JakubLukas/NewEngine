#include "../os_utils.h"

#include <core/os/win/simple_windows.h>
//#include <dbghelp.h>

#include "core/asserts.h"
#include "core/string.h"


namespace Veng
{


namespace os
{


struct WorkingDir
{
	WorkingDir()
	{
		GetCurrentDirectory(MAX_PATH, dir);

		char* str = dir;
		while(str[0] != '\0')
		{
			if(str[0] == '\\')
				str[0] = '/';
			str++;
		}
	}

	char dir[MAX_PATH];
};
static WorkingDir workingDir;


void GetWorkingDir(char* path, size_t maxLen)
{
	memcpy(path, workingDir.dir, (maxLen < MAX_PATH) ? maxLen : MAX_PATH);
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


bool ShowOpenFileDialog(FileDialogData& data)
{
	const size_t MAX_FILTER = 1024 + 1;

	size_t filterLen = string::Length(data.filter);
	if (filterLen >= MAX_FILTER - 1)
	{
		ASSERT2(false, "Filter is too long");
		return false;
	}
	char filterBuffer[MAX_FILTER] = {0};
	for (int i = 0; i < filterLen; ++i)
	{
		if (data.filter[i] == ',')
			filterBuffer[i] = '\0';
		else
			filterBuffer[i] = data.filter[i];
	}

	bool result = false;

	OPENFILENAME dialogInfo = { 0 };
	dialogInfo.lStructSize = sizeof(OPENFILENAME);
	dialogInfo.hwndOwner = (HWND)data.parentWindow;
	dialogInfo.hInstance;//ignored
	dialogInfo.lpstrFilter = filterBuffer;
	dialogInfo.lpstrCustomFilter;//ignored
	dialogInfo.nMaxCustFilter;//ignored
	dialogInfo.nFilterIndex = data.filterIndex+1;
	dialogInfo.lpstrFile = data.fileName;
	dialogInfo.nMaxFile = MAX_PATH_LENGTH;
	dialogInfo.lpstrFileTitle = NULL;//???
	dialogInfo.nMaxFileTitle;//^?^?^?
	dialogInfo.lpstrInitialDir = data.initDir;
	dialogInfo.lpstrTitle = data.title;
	if (GetOpenFileName(&dialogInfo) != 0)
	{
		data.filterIndex--;
		data.fileOffset = dialogInfo.nFileOffset;
		data.extensionOffset = dialogInfo.nFileExtension;
		result = true;
	}
	else
	{
		DWORD err = CommDlgExtendedError();
		if (err != 0)//err 0 is "user closed or canceled the dialog box"
			ASSERT2(false, "Failed to call GetOpenFileName");
		result = false;
	}

	::SetCurrentDirectory(workingDir.dir);

	return result;
}

bool ShowSaveFileDialog(FileDialogData& data)
{
	const size_t MAX_FILTER = 1024 + 1;

	size_t filterLen = string::Length(data.filter);
	if (filterLen >= MAX_FILTER - 1)
	{
		ASSERT2(false, "Filter is too long");
		return false;
	}
	char filterBuffer[MAX_FILTER] = { 0 };
	for (int i = 0; i < filterLen; ++i)
	{
		if (data.filter[i] == ',')
			filterBuffer[i] = '\0';
		else
			filterBuffer[i] = data.filter[i];
	}

	bool result = false;

	OPENFILENAME dialogInfo = { 0 };
	dialogInfo.lStructSize = sizeof(OPENFILENAME);
	dialogInfo.hwndOwner = (HWND)data.parentWindow;
	dialogInfo.hInstance;//ignored
	dialogInfo.lpstrFilter = filterBuffer;
	dialogInfo.lpstrCustomFilter;//ignored
	dialogInfo.nMaxCustFilter;//ignored
	dialogInfo.nFilterIndex = data.filterIndex + 1;
	dialogInfo.lpstrFile = data.fileName;
	dialogInfo.nMaxFile = MAX_PATH_LENGTH;
	dialogInfo.lpstrFileTitle = NULL;//???
	dialogInfo.nMaxFileTitle;//^?^?^?
	dialogInfo.lpstrInitialDir = data.initDir;
	dialogInfo.lpstrTitle = data.title;

	if (GetSaveFileName(&dialogInfo) != 0)
	{
		data.filterIndex--;
		data.fileOffset = dialogInfo.nFileOffset;
		data.extensionOffset = dialogInfo.nFileExtension;
		result = true;
	}
	else
	{
		DWORD err = CommDlgExtendedError();
		if(err != 0)//err 0 is "user closed or canceled the dialog box"
			ASSERT2(false, "Failed to call GetOpenFileName");
		result = false;
	}

	::SetCurrentDirectory(workingDir.dir);

	return result;
}


void PathToNativePath(char* path)
{
	while(*path != '\0')
	{
		if(*path == '/') *path = '\\';
		path++;
	}
}

void PathToEnginePath(char* path)
{
	while(*path != '\0')
	{
		if(*path == '\\') *path = '/';
		path++;
	}
}



u32 GetCallStack(u32 framesToSkip, u32 framesToCapture, void** callstack)
{
	return ::RtlCaptureStackBackTrace(
		(ULONG)framesToSkip,
		(ULONG)framesToCapture,
		(PVOID*)callstack,
		NULL
	);
}

/*osHandle GetCurrentProcessHandle()
{
	return ::GetCurrentProcess();
}

bool InitSymbols(osHandle process)
{
	if(::SymInitialize(process, NULL, true) == TRUE)
	{
		return true;
	}
	else
	{
		ASSERT2(false, "Function call failed, call GetLastError");
		return false;
	}
}

bool LoadSymbols(osHandle process/*ImageName, ModuleName, BaseOfDll, SizeOfDll/)
{
	if(SymLoadModule(
		process,
		NULL,
		"",
		NULL,
		0,
		0
	) != 0)
	{
		return true;
	}
	else
	{
		ASSERT2(false, "Function call failed, call GetLastError");
		return false;
	}
}*/


void GetMouseCursorPos(int& x, int& y)
{
	POINT p;
	if (::GetCursorPos(&p) != 0)
	{
		x = p.x;
		y = p.y;
	}
	else
	{
		DWORD errId = ::GetLastError();
		LogErrorMessage(errId);
		ASSERT2(false, "GetCursorPos failed");
	}
}

void SetMouseCursorPos(int x, int y)
{
	if (::SetCursorPos(x, y) == 0)
	{
		DWORD errId = ::GetLastError();
		LogErrorMessage(errId);
		ASSERT2(false, "SetCursorPos failed");
	}
}

void ShowMouseCursor(bool show)
{
	::ShowCursor(show);
}


}


}
