#include "asserts.h"

#include "os/os_utils.h"


namespace Veng
{


void MyDebugBreak()
{
	//__debugbreak();
}

void MyOutputDebugString(const char* text)
{
	os::LogDebugString(text);
}


}