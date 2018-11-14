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

	ProcessorArchitecture processorArchitecture = ProcessorArchitecture::Unknown;
	u32 pageSize = 0;
	void* minimumAddress = nullptr;
	void* maximumAddress = nullptr;
	u32 activeProcessorMask = 0;
	u32 numberOfProcessors = 0;
	u32 allocationGranularity = 0;
};


void GetCurrentDir(char* path, size_t maxLen);

void LogDebugString(const char* str);

int CallProcess(const char* appPath, char* args);

SystemInfo GetSystemInfo();


}


}