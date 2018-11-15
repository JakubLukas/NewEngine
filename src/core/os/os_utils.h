#pragma once

#include "core/int.h"


namespace Veng
{


namespace os
{


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


}


}