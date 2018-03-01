#pragma once


namespace Veng
{


namespace os
{


void GetCurrentDir(char* path, size_t maxLen);

void LogDebugString(const char* str);

void CallProcess(const char* appPath, char* args);


}


}