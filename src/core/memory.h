#pragma once


namespace Veng
{


namespace memory
{


void Copy(void* destination, const void* source, size_t size);

void Move(void* destination, const void* source, size_t size);

void Set(void* destination, unsigned char value, size_t size);

void Swap(void* source1, void* source2, size_t size);


}


}