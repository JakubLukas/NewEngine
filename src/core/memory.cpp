#include "memory.h"

#include <cstring>


namespace Veng
{


void MemCpy(void* destination, const void* source, size_t size)
{
	memcpy(destination, source, size);
}


void MemMove(void* destination, const void* source, size_t size)
{
	memmove(destination, source, size);
}

void MemSet(void* destination, unsigned char value, size_t size)
{
	memset(destination, value, size);
}


}