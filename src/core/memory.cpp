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

void Swap(void* source1, void* source2, size_t size)
{
	char* src1 = (char*)source1;
	char* src2 = (char*)source2;
	char tmp;

	for (size_t i = 0; i < size; ++i)
	{
		tmp = *src1;
		*src1 = *src2;
		*src2 = tmp;
		src1++;
		src2++;
	}
}


}


}