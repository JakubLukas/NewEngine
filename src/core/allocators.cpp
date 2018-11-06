#include "allocators.h"
#include <cstdlib>

#include "math/math.h"


/*void* operator new(size_t)
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
}*/


namespace Veng
{


void* AlignPointer(void* ptr, size_t alignment)
{
	return (void*)(((uintptr)ptr + (uintptr)alignment - 1) & ~(uintptr)(alignment - 1));
}


// ---------------- MAIN ALLOCATOR ----------------


MainAllocator::MainAllocator()
{
}

MainAllocator::~MainAllocator()
{
	ASSERT2(m_allocCount == 0, "Memory leak");
	ASSERT2(m_allocSize == 0, "Memory leak");
}

void* MainAllocator::Allocate(size_t size, size_t alignment)
{
	m_allocCount++;
	size_t allocAlign = Max(alignment, ALIGN_OF(AllocationInfo));
	size_t allocSize = allocAlign + sizeof(AllocationInfo) + alignment + size + ALIGN_OF(uintptr) + sizeof(uintptr);
	m_allocSize += allocSize;
	void* ptr = malloc(allocSize);
	void* data = AlignPointer((char*)ptr + sizeof(AllocationInfo), allocAlign);
	ASSERT2((uintptr)data % alignment == 0, "Invalid alignment");
	AllocationInfo* info = static_cast<AllocationInfo*>((void*)((char*)data - sizeof(AllocationInfo)));
	ASSERT2((uintptr)info % ALIGN_OF(AllocationInfo) == 0, "Invalid alignment");
	info->size = size;
	info->allSize = allocSize;
	uintptr* startPtr = static_cast<uintptr*>(AlignPointer((char*)data + size, ALIGN_OF(uintptr)));
	ASSERT2((uintptr)startPtr % ALIGN_OF(uintptr) == 0, "Invalid alignment");
	*startPtr = reinterpret_cast<uintptr>(ptr);
	return data;
}

void* MainAllocator::Reallocate(void* ptr, size_t size, size_t alignment)
{
	AllocationInfo* origInfo = static_cast<AllocationInfo*>((void*)((char*)ptr - sizeof(AllocationInfo)));
	uintptr* origPtr = static_cast<uintptr*>(AlignPointer((char*)ptr + origInfo->size, ALIGN_OF(uintptr)));
	size_t allocAlign = Max(alignment, ALIGN_OF(AllocationInfo));
	size_t allocSize = allocAlign + sizeof(AllocationInfo) + alignment + size + ALIGN_OF(uintptr) + sizeof(uintptr);
	m_allocSize += allocSize - origInfo->allSize;
	void* newptr = realloc((void*)*origPtr, allocSize);
	void* data = AlignPointer((char*)newptr + sizeof(AllocationInfo), allocAlign);
	ASSERT2((uintptr)data % alignment == 0, "Invalid alignment");
	AllocationInfo* info = static_cast<AllocationInfo*>((void*)((char*)data - sizeof(AllocationInfo)));
	ASSERT2((uintptr)info % ALIGN_OF(AllocationInfo) == 0, "Invalid alignment");
	info->size = size;
	info->allSize = allocSize;
	uintptr* startPtr = static_cast<uintptr*>(AlignPointer((char*)data + size, ALIGN_OF(uintptr)));
	ASSERT2((uintptr)startPtr % ALIGN_OF(uintptr) == 0, "Invalid alignment");
	*startPtr = reinterpret_cast<uintptr>(newptr);
	return data;
}

void MainAllocator::Deallocate(void* ptr)
{
	ASSERT(ptr != nullptr);

	AllocationInfo* origInfo = static_cast<AllocationInfo*>((void*)((char*)ptr - sizeof(AllocationInfo)));
	uintptr* origPtr = static_cast<uintptr*>(AlignPointer((char*)ptr + origInfo->size, ALIGN_OF(uintptr)));

	m_allocCount--;
	m_allocSize -= origInfo->allSize;
	free((void*)*origPtr);
}

size_t MainAllocator::GetSize(void* ptr) const
{
	AllocationInfo* info = static_cast<AllocationInfo*>((void*)((char*)ptr - sizeof(AllocationInfo)));
	return info->size;
}

const char* MainAllocator::GetName() const { return "Main"; }

i64 MainAllocator::GetAllocCount() const { return m_allocCount; }

size_t MainAllocator::GetAllocSize() const { return m_allocSize; }


// ---------------- HEAP ALLOCATOR ----------------


HeapAllocator::HeapAllocator(IAllocator& allocator)
	: m_source(allocator)
{ }

HeapAllocator::~HeapAllocator()
{
	ASSERT2(m_allocCount == 0, "Memory leak");
	ASSERT2(m_allocSize == 0, "Memory leak");
}

void* HeapAllocator::Allocate(size_t size, size_t alignment)
{
	m_allocCount++;
	m_allocSize += size;
	return m_source.Allocate(size, alignment);
}

void* HeapAllocator::Reallocate(void* ptr, size_t size, size_t alignment)
{
	m_allocSize += size - m_source.GetSize(ptr);
	return m_source.Reallocate(ptr, size, alignment);
}

void HeapAllocator::Deallocate(void* ptr)
{
	ASSERT(ptr != nullptr);
	m_allocCount--;
	m_allocSize -= m_source.GetSize(ptr);
	m_source.Deallocate(ptr);
}

size_t HeapAllocator::GetSize(void* ptr) const
{
	return m_source.GetSize(ptr);
}

void HeapAllocator::SetName(const char* name) { m_name = name; }

const char* HeapAllocator::GetName() const { return m_name; }

i64 HeapAllocator::GetAllocCount() const { return m_allocCount; }

size_t HeapAllocator::GetAllocSize() const { return m_allocSize; }


}


void* operator new(size_t, Veng::NewPlaceholder, void* where)
{
	return where;
}


void* operator new(size_t size, Veng::IAllocator& allocator, size_t alignment)
{
	return allocator.Allocate(size, alignment);
}