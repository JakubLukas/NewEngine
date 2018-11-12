#include "allocators.h"
#include <cstdlib>

#include <windows.h>///////////////////////////////////////////

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


#if DEBUG_ALLOCATORS

struct AllocHeader
{
	size_t size;
	size_t allSize;
	void* ptr;
};


struct SystemInfo
{
	SystemInfo()
	{
		SYSTEM_INFO sysInfo;
		::GetSystemInfo(&sysInfo);

		minAddress = sysInfo.lpMinimumApplicationAddress;
		maxAddress = sysInfo.lpMaximumApplicationAddress;
		allocationGranularity = sysInfo.dwAllocationGranularity;
		pageSize = sysInfo.dwPageSize;
	}

	void* minAddress;
	void* maxAddress;
	size_t allocationGranularity;
	size_t pageSize;
};
static SystemInfo s_systemInfo;


void ArrayInit(AllocArray& arr, class IAllocator& allocator)
{
	arr.capacity = 64;
	arr.data = (void**)allocator.Allocate(arr.capacity * sizeof(void*), alignof(void*));
}

void ArrayDeinit(AllocArray& arr, class IAllocator& allocator)
{
	allocator.Deallocate(arr.data);
	arr.data = nullptr;
	arr.size = 0;
	arr.capacity = 0;
}

void ArrayCheckSize(AllocArray& arr, class IAllocator& allocator)
{
	if (arr.size == arr.capacity)
	{
		arr.capacity *= 2;
		arr.data = (void**)allocator.Reallocate(arr.data, arr.capacity * sizeof(void*), alignof(void*));
	}
}

void ArrayAddOrdered(AllocArray& arr, void* elem)
{
	ASSERT2(arr.size < arr.capacity, "Not enough memory in array");

	for (size_t i = 0; i < arr.size; ++i)
	{
		if (arr.data[i] > elem)
		{
			memory::Move(arr.data + i + 1, arr.data + i, (arr.size - i) * sizeof(void*));
			arr.data[i] = elem;
			arr.size++;
			return;
		}
	}
	arr.data[arr.size++] = elem;
}

void ArrayEraseOrdered(AllocArray& arr, void* elem)
{
	for (size_t i = 0; i < arr.size; ++i)
	{
		if (arr.data[i] == elem)
		{
			memory::Move(arr.data + i, arr.data + i + 1, (arr.size - i) * sizeof(void*));
			arr.size--;
			return;
		}
	}
}

bool ArrayReplace(AllocArray& arr, void* oldElem, void* newElem)
{
	for (size_t i = 0; i < arr.size; ++i)
	{
		if (arr.data[i] == oldElem)
		{
			arr.data[i] = newElem;
			return true;
		}
	}
	return false;
}

bool ArrayFind(AllocArray& arr, void* elem, void*& arrElem)
{
	for (size_t i = 0; i < arr.size; ++i)
	{
		if (arr.data[i] == elem)
		{
			arrElem = arr.data[i];
			return true;
		}
	}
	return false;
}

#endif


// ---------------- MAIN ALLOCATOR ----------------


MainAllocator::MainAllocator()
{
}

MainAllocator::~MainAllocator()
{
#if DEBUG_ALLOCATORS
	ASSERT2(m_allocCount == 0, "Memory leak");
	ASSERT2(m_allocSize == 0, "Memory leak");
#endif
}

void* MainAllocator::Allocate(size_t size, size_t alignment)
{
#if DEBUG_ALLOCATORS
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
#if DEBUG_ALLOCATORS
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

#if DEBUG_ALLOCATORS
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
#if DEBUG_ALLOCATORS
	AllocHeader* info = static_cast<AllocHeader*>((void*)((char*)ptr - sizeof(AllocHeader)));
	return info->size;
#else
	return 0;
#endif
}

const char* MainAllocator::GetName() const{ return "Main"; }

i64 MainAllocator::GetAllocCount() const
{
#if DEBUG_ALLOCATORS
	return m_allocCount;
#else
	return 0;
#endif
}

size_t MainAllocator::GetAllocSize() const
{
#if DEBUG_ALLOCATORS
	return m_allocSize;
#else
	return 0;
#endif
}


// ---------------- HEAP ALLOCATOR ----------------


HeapAllocator::HeapAllocator(IAllocator& allocator)
	: m_source(allocator)
{
#if DEBUG_ALLOCATORS
	ArrayInit(m_allocations, m_source);
	ArrayInit(m_pages, m_source);
	ArrayInit(m_pagesCounts, m_source);
#endif
}

HeapAllocator::~HeapAllocator()
{
#if DEBUG_ALLOCATORS
	ArrayDeinit(m_pages, m_source);
	ArrayDeinit(m_pagesCounts, m_source);
	ArrayDeinit(m_allocations, m_source);

	ASSERT2(m_allocCount == 0, "Memory leak");
	ASSERT2(m_allocSize == 0, "Memory leak");
#endif
}

void* HeapAllocator::Allocate(size_t size, size_t alignment)
{
	void* data = m_source.Allocate(size, alignment);
#if DEBUG_ALLOCATORS
	m_allocCount++;
	m_allocSize += size;

	ArrayCheckSize(m_allocations, m_source);
	ArrayAddOrdered(m_allocations, data);

	void* pagePtr = (void*)(((uintptr)data / s_systemInfo.pageSize) * s_systemInfo.pageSize);//TODO: bit operations
	void* arrPagePtr = nullptr;
	if (!ArrayFind(m_pages, pagePtr, arrPagePtr))
	{
		ArrayCheckSize(m_pages, m_source);
		ArrayAddOrdered(m_pages, pagePtr);
		ArrayCheckSize(m_pagesCounts, m_source);
		ArrayAddOrdered(m_pagesCounts, (void*)0);
	}
	else
	{
		arrPagePtr = (void*)((uintptr)arrPagePtr + 1);
	}
#endif
	return data;
}

void* HeapAllocator::Reallocate(void* ptr, size_t size, size_t alignment)
{
#if DEBUG_ALLOCATORS
	m_allocSize += size - m_source.GetSize(ptr);
#endif

	void* data = m_source.Reallocate(ptr, size, alignment);

#if DEBUG_ALLOCATORS
	if (ptr == nullptr)
	{
		ArrayCheckSize(m_allocations, m_source);
		ArrayAddOrdered(m_allocations, data);
	}
	else
	{
		ArrayReplace(m_allocations, ptr, data);
	}
	
	void* pagePtr = (void*)((uintptr)data / s_systemInfo.pageSize);
	void* arrPagePtr = nullptr;
	if (!ArrayFind(m_pages, pagePtr, arrPagePtr))
	{
		ArrayCheckSize(m_pages, m_source);
		ArrayAddOrdered(m_pages, pagePtr);
	}
	else
	{
		//ASSERT(ArrayFind(m_pagesCounts, ), "");
		arrPagePtr = (void*)((uintptr)arrPagePtr + 1);
	}
#endif

	return data;
}

void HeapAllocator::Deallocate(void* ptr)
{
	ASSERT(ptr != nullptr);
#if DEBUG_ALLOCATORS
	m_allocCount--;
	m_allocSize -= m_source.GetSize(ptr);

	ArrayEraseOrdered(m_allocations, ptr);

	void* pagePtr = (void*)((uintptr)ptr / s_systemInfo.pageSize);
	void* arrPagePtr = nullptr;
	if (ArrayFind(m_pages, pagePtr, arrPagePtr))
	{

	}
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
#if DEBUG_ALLOCATORS
	return m_allocCount;
#else
	return 0;
#endif
}

size_t HeapAllocator::GetAllocSize() const
{
#if DEBUG_ALLOCATORS
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