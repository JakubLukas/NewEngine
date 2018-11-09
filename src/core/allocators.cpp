#include "allocators.h"
#include <cstdlib>

#include "math/math.h"


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
	return (void*)(((uintptr)ptr + (uintptr)alignment - 1) & ~(uintptr)(alignment - 1));
}


#if defined(DEBUG) || DEBUG_ALLOCATORS

struct AllocHeader
{
	size_t size;
	size_t allSize;
	void* ptr;
};



#endif


// ---------------- MAIN ALLOCATOR ----------------


MainAllocator::MainAllocator()
{
}

MainAllocator::~MainAllocator()
{
#if defined(DEBUG) || DEBUG_ALLOCATORS
	ASSERT2(m_allocCount == 0, "Memory leak");
	ASSERT2(m_allocSize == 0, "Memory leak");
#endif
}

void* MainAllocator::Allocate(size_t size, size_t alignment)
{
#if defined(DEBUG) || DEBUG_ALLOCATORS
	size_t allocAlign = Max(alignment, alignof(AllocHeader));
	size_t allocSize = allocAlign + sizeof(AllocHeader) + alignment + size;
	void* ptr = malloc(allocSize);
	void* data = AlignPointer((char*)ptr + sizeof(AllocHeader), allocAlign);
	ASSERT2((uintptr)data % alignment == 0, "Invalid alignment");
	AllocHeader* header = static_cast<AllocHeader*>((void*)((char*)data - sizeof(AllocHeader)));
	ASSERT2((uintptr)header % alignof(AllocHeader) == 0, "Invalid alignment");
	header->size = size;
	header->allSize = allocSize;
	header->ptr = ptr;

	m_allocCount++;
	m_allocSize += allocSize;
	return data;
#else
	void* ptr = malloc(alignment + size);
	void* data = AlignPointer((char*)ptr + 1, alignment);
	*((char*)data - 1) = (char)((ptrdiff_t)data - (ptrdiff_t)ptr);
	return data;
#endif
}

void* MainAllocator::Reallocate(void* ptr, size_t size, size_t alignment)
{
#if defined(DEBUG) || DEBUG_ALLOCATORS
	AllocHeader origHeader = *static_cast<AllocHeader*>((void*)((char*)ptr - sizeof(AllocHeader)));

	size_t allocAlign = Max(alignment, alignof(AllocHeader));
	size_t allocSize = allocAlign + sizeof(AllocHeader) + alignment + size;
	void* newptr = realloc((void*)origHeader.ptr, allocSize);
	//if realloc returns memory pointer with different alignment as malloc, whole universe will explode (data pointer would points to wrong memory)
	void* data = AlignPointer((char*)newptr + sizeof(AllocHeader), allocAlign);
	ASSERT2((uintptr)data % alignment == 0, "Invalid alignment");
	AllocHeader* header = static_cast<AllocHeader*>((void*)((char*)data - sizeof(AllocHeader)));
	ASSERT2((uintptr)header % alignof(AllocHeader) == 0, "Invalid alignment");
	header->size = size;
	header->allSize = allocSize;
	header->ptr = newptr;

	m_allocSize += allocSize - origHeader.allSize;
	return data;
#else
	void* origPtr = (char*)ptr - *((char*)ptr - 1);
	void* newPtr = realloc(origPtr, alignment + size);
	//if realloc returns memory pointer with different alignment as malloc, whole universe will explode (data pointer would points to wrong memory)
	void* data = AlignPointer((char*)newPtr + 1, alignment);
	*((char*)data - 1) = (char)((ptrdiff_t)data - (ptrdiff_t)newPtr);
	return data;
#endif
}

void MainAllocator::Deallocate(void* ptr)
{
	ASSERT(ptr != nullptr);

#if defined(DEBUG) || DEBUG_ALLOCATORS
	AllocHeader* header = static_cast<AllocHeader*>((void*)((char*)ptr - sizeof(AllocHeader)));
	m_allocCount--;
	m_allocSize -= header->allSize;
	free(header->ptr);
#else
	void* origPtr = (char*)ptr - *((char*)ptr - 1);
	free(origPtr);
#endif
}

size_t MainAllocator::GetSize(void* ptr) const
{
#if defined(DEBUG) || DEBUG_ALLOCATORS
	AllocHeader* info = static_cast<AllocHeader*>((void*)((char*)ptr - sizeof(AllocHeader)));
	return info->size;
#else
	return 0;
#endif
}

const char* MainAllocator::GetName() const{ return "Main"; }

i64 MainAllocator::GetAllocCount() const
{
#if defined(DEBUG) || DEBUG_ALLOCATORS
	return m_allocCount;
#else
	return 0;
#endif
}

size_t MainAllocator::GetAllocSize() const
{
#if defined(DEBUG) || DEBUG_ALLOCATORS
	return m_allocSize;
#else
	return 0;
#endif
}


// ---------------- HEAP ALLOCATOR ----------------


HeapAllocator::HeapAllocator(IAllocator& allocator)
	: m_source(allocator)
{
	m_allocationsCapacity = 64;
	m_allocations = (void**)m_source.Allocate(m_allocationsCapacity * sizeof(void*), alignof(void*));
}

HeapAllocator::~HeapAllocator()
{
	m_source.Deallocate(m_allocations);

#if defined(DEBUG) || DEBUG_ALLOCATORS
	ASSERT2(m_allocCount == 0, "Memory leak");
	ASSERT2(m_allocSize == 0, "Memory leak");
#endif
}

void* HeapAllocator::Allocate(size_t size, size_t alignment)
{
	void* data = m_source.Allocate(size, alignment);
#if defined(DEBUG) || DEBUG_ALLOCATORS
	m_allocCount++;
	m_allocSize += size;
	//add to array
	if (m_allocationsSize == m_allocationsCapacity)
	{
		m_allocationsCapacity *= 2;
		m_allocations = (void**)m_source.Reallocate(m_allocations, m_allocationsCapacity * sizeof(void*), alignof(void*));
	}
	m_allocations[m_allocationsSize++] = data;
	m_allocationsMin = Min(m_allocationsMin, (uintptr)data);
	m_allocationsMax = Max(m_allocationsMax, (uintptr)data + size);
#endif
	return data;
}

void* HeapAllocator::Reallocate(void* ptr, size_t size, size_t alignment)
{
#if defined(DEBUG) || DEBUG_ALLOCATORS
	m_allocSize += size - m_source.GetSize(ptr);
#endif

	void* data = m_source.Reallocate(ptr, size, alignment);

#if defined(DEBUG) || DEBUG_ALLOCATORS
	//change in array
	for (size_t i = 0; i < m_allocationsSize; ++i)
	{
		if (m_allocations[i] == ptr)
		{
			m_allocations[i] = data;
			break;
		}
	}
	m_allocationsMin = Min(m_allocationsMin, (uintptr)data);
	m_allocationsMax = Max(m_allocationsMax, (uintptr)data + size);
#endif

	return data;
}

void HeapAllocator::Deallocate(void* ptr)
{
	ASSERT(ptr != nullptr);
#if defined(DEBUG) || DEBUG_ALLOCATORS
	m_allocCount--;
	m_allocSize -= m_source.GetSize(ptr);
	//remove from array
	/*for (size_t i = 0; i < m_allocationsSize; ++i)
	{
		if (m_allocations[i] == ptr)
		{
			m_allocationsSize--;
			m_allocations[i] = m_allocations[m_allocationsSize];
			break;
		}
	}*/
	//shrink m_allocationsMin and m_allocationsMax
#endif
	m_source.Deallocate(ptr);
}

size_t HeapAllocator::GetSize(void* ptr) const
{
	return m_source.GetSize(ptr);
}

void HeapAllocator::SetName(const char* name) { m_name = name; }

const char* HeapAllocator::GetName() const { return m_name; }

i64 HeapAllocator::GetAllocCount() const
{
#if defined(DEBUG) || DEBUG_ALLOCATORS
	return m_allocCount;
#else
	return 0;
#endif
}

size_t HeapAllocator::GetAllocSize() const
{
#if defined(DEBUG) || DEBUG_ALLOCATORS
	return m_allocSize;
#else
	return 0;
#endif
}


}


void* operator new(size_t, Veng::NewPlaceholder, void* where)
{
	return where;
}


void* operator new(size_t size, Veng::IAllocator& allocator, size_t alignment)
{
	return allocator.Allocate(size, alignment);
}