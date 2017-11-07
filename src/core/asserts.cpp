#include <windows.h>


void MyDebugBreak()
{
	__debugbreak();
}

void MyOutputDebugString(const char* text)
{
	OutputDebugString(text);
}