#pragma once

#include "core/int.h"


namespace Veng
{


namespace os
{

typedef void* osHandle;
const size_t MAX_PATH_LENGTH = 260;


struct SystemInfo
{
	enum class ProcessorArchitecture : u32
	{
		Unknown,
		Intel,
		IA64,
		AMD64,
		ARM,
		ARM64,
	};

	ProcessorArchitecture processorArchitecture;
	u32 pageSize;
	void* minimumAddress;
	void* maximumAddress;
	u32 activeProcessorMask;
	u32 numberOfProcessors;
	u32 allocationGranularity;
};


struct FileDialogData
{
	osHandle parentWindow = nullptr;//handle of parent window, can be null
	const char* filter = "All Files,*.*";//comma separated pairs of [name,pattern]. pattern can specify multiple filters, separated by semicolon
	u32 filterIndex = 0;//after use, contains index of filter selected by user
	char fileName[MAX_PATH_LENGTH] = {0};//after use, contains full path of file selected
	char initDir[MAX_PATH_LENGTH] = {0};//initial directory, if null, current directory is used
	char* title = nullptr;//title of dialog window, if null, "Save As" or "Open" is used
	//return values under
	u16 fileOffset;//offset of file name in full path stored in fileName;
	u16 extensionOffset;//offset of file extension in full path stored in fileName; points to string after last occurrence of '.'; if no '.' in string -> it's zero
};


void GetWorkingDir(char* path, size_t maxLen);

void LogDebugString(const char* str);

int CallProcess(const char* appPath, char* args);

SystemInfo GetSystemInfo();


//custom template, custom filter, multiselect | are not supported
bool ShowOpenFileDialog(FileDialogData& data);

bool ShowSaveFileDialog(FileDialogData& data);

void PathToNativePath(char* path);
void PathToEnginePath(char* path);


u32 GetCallStack(u32 framesToSkip, u32 framesToCapture, void** callstack);

//http://www.debuginfo.com/examples/src/SymLoadPdb.cpp
//osHandle GetCurrentProcessHandle();
//
//bool InitSymbols(osHandle process);
//
//bool LoadSymbols(osHandle process/*ImageName, ModuleName, BaseOfDll, SizeOfDll*/);


void GetMouseCursorPos(int& x, int& y);
void SetMouseCursorPos(int x, int y);
void ShowMouseCursor(bool show);

}


}