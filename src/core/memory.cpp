#include "memory.h"

#include <cstring>


namespace Veng
{


namespace memory
{


void Copy(void* destination, const void* source, size_t size)
{
	memcpy(destination, source, size);
}


void Move(void* destination, const void* source, size_t size)
{
	memmove(destination, source, size);
}

void Set(void* destination, unsigned char value, size_t size)
{
	memset(destination, value, size);
}


}


}