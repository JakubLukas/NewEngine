#pragma once

#include "core/int.h"


namespace Veng
{


namespace os
{

typedef void* osHandle;


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


void GetCurrentDir(char* path, size_t maxLen);

void LogDebugString(const char* str);

int CallProcess(const char* appPath, char* args);

SystemInfo GetSystemInfo();


u32 GetCallStack(u32 framesToSkip, u32 framesToCapture, void** callstack);

//http://www.debuginfo.com/examples/src/SymLoadPdb.cpp
//osHandle GetCurrentProcessHandle();
//
//bool InitSymbols(osHandle process);
//
//bool LoadSymbols(osHandle process/*ImageName, ModuleName, BaseOfDll, SizeOfDll*/);


}


}