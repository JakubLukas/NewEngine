#pragma once


namespace Veng
{


void MemCpy(void* destination, const void* source, size_t size);

void MemMove(void* destination, const void* source, size_t size);

void MemSet(void* destination, unsigned char value, size_t size);


}