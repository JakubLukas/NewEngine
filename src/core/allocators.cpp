#include "allocators.h"
#include <cstdlib>


void* operator new(size_t)
{
	ASSERT2(false, "Usage of new is forbidden, use placement new instead");
	return nullptr;
}

void operator delete(void*)
{
	ASSERT2(false, "Usage of new is forbidden, use placement new instead");
}


void* operator new[](size_t)
{
	ASSERT2(false, "Usage of new is forbidden, use placement new instead");
	return nullptr;
}

void operator delete[](void*)
{
	ASSERT2(false, "Usage of new is forbidden, use placement new instead");
}


namespace Veng
{


void* AlignPointer(void* ptr, size_t alignment)
{
	return (void*)(((uintptr)ptr + (uintptr)alignment) & ~(uintptr)alignment);
}

// ---------------- MAIN ALLOCATOR ----------------


MainAllocator::MainAllocator()
	: m_allocCount(0)
{
}

MainAllocator::~MainAllocator()
{
	ASSERT(m_allocCount == 0);
}

void* MainAllocator::Allocate(size_t size, size_t alignment)
{
	++m_allocCount;
	return _aligned_malloc(size, alignment);
}

void* MainAllocator::Reallocate(void* ptr, size_t size, size_t alignment)
{
	return _aligned_realloc(ptr, size, alignment);
}

void MainAllocator::Deallocate(void* p)
{
	ASSERT(p != nullptr);
	--m_allocCount;
	_aligned_free(p);
}

size_t MainAllocator::AllocatedSize(void* p)
{
	return 0;//TODO: fix
}


// ---------------- HEAP ALLOCATOR ----------------


HeapAllocator::HeapAllocator(IAllocator& allocator)
	: m_source(allocator)
	, m_allocCount(0)
{

}

HeapAllocator::~HeapAllocator()
{
	ASSERT(m_allocCount == 0);
}

void* HeapAllocator::Allocate(size_t size, size_t alignment)
{
	++m_allocCount;
	return m_source.Allocate(size, alignment);
}

void* HeapAllocator::Reallocate(void* ptr, size_t size, size_t alignment)
{
	return m_source.Reallocate(ptr, size, alignment);
}

void HeapAllocator::Deallocate(void* p)
{
	ASSERT(p != nullptr);
	--m_allocCount;
	m_source.Deallocate(p);
}

size_t HeapAllocator::AllocatedSize(void* p)
{
	return m_source.AllocatedSize(p);
}


}


void* operator new(size_t size, Veng::NewPlaceholder, void* where)
{
	return where;
}


void* operator new(size_t size, Veng::IAllocator& allocator, size_t alignment)
{
	return allocator.Allocate(size, alignment);
}