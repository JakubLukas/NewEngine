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


// ---------------- MAIN ALLOCATOR ----------------

MainAllocator::MainAllocator()
{
}

MainAllocator::~MainAllocator()
{
}

void* MainAllocator::Allocate(size_t size)
{

	size_t* p = static_cast<size_t*>(malloc(sizeof(size_t) + size));
	*p = size;
	return &p[1];
}

void* MainAllocator::Reallocate(void* ptr, size_t size)
{

	size_t* p = static_cast<size_t*>(realloc(static_cast<size_t*>(ptr) - 1, sizeof(size_t) + size));
	*p = size;
	return &p[1];
}

void MainAllocator::Deallocate(void* p)
{
	if (p == nullptr) return;
	free(static_cast<size_t*>(p) - 1);
}

size_t MainAllocator::AllocatedSize(void* p)
{
	return static_cast<size_t*>(p)[-1];
}


// ---------------- HEAP ALLOCATOR ----------------

HeapAllocator::HeapAllocator(IAllocator& allocator)
	: m_allocator(allocator)
{

}

HeapAllocator::~HeapAllocator()
{

}

void* HeapAllocator::Allocate(size_t size)
{
	return m_allocator.Allocate(size);
}

void* HeapAllocator::Reallocate(void* ptr, size_t size)
{
	return m_allocator.Reallocate(ptr, size);
}

void HeapAllocator::Deallocate(void* p)
{
	m_allocator.Deallocate(p);
}

size_t HeapAllocator::AllocatedSize(void* p)
{
	return m_allocator.AllocatedSize(p);
}


}