#pragma once


namespace Veng
{

enum class LogType : char
{
	Info,
	Warning,
	Error,
};

void Log(LogType type, const char* format, ...);

void Log(LogType type, const char* format, char* args);


}