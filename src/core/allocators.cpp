#include "allocators.h"
#include "asserts.h"
#include <malloc.h>


void* operator new(size_t)
{
	ASSERT2(false, "Use of new is forbidden");
	return nullptr;
}

void operator delete(void*)
{
	ASSERT2(false, "Use of new is forbidden");
}


void* operator new[](size_t)
{
	ASSERT2(false, "Use of new is forbidden");
	return nullptr;
}

void operator delete[](void*)
{
	ASSERT2(false, "Use of new is forbidden");
}


void* operator new(size_t size, NewPlaceholder, void* where)
{
	return where;
}



namespace Veng
{


void* HeapAllocator::Allocate(size_t size)
{

	size_t* p = static_cast<size_t*>(malloc(sizeof(size_t) + size));
	*p = size;
	return &p[1];
}


void HeapAllocator::Deallocate(void* p)
{
	free(static_cast<size_t*>(p) - 1);
}


size_t HeapAllocator::AllocatedSize(void* p)
{
	return static_cast<size_t*>(p)[-1];
}


}