#include "allocators.h"

#include <cstdlib>

#include "math/math.h"
#include "core/os/os_utils.h"


namespace Veng
{


#if DEBUG_ALLOCATORS

struct AllocHeader
{
	size_t size;
	size_t allSize;
	void* ptr;
};


struct AllocInfo
{
	AllocInfo()
	{
		const os::SystemInfo sysInfo = os::GetSystemInfo();

		minAddress = sysInfo.minimumAddress;
		maxAddress = sysInfo.maximumAddress;
		allocationGranularity = sysInfo.allocationGranularity;
		pageSize = sysInfo.pageSize;
	}

	void* minAddress;
	void* maxAddress;
	size_t allocationGranularity;
	size_t pageSize;
};

static const AllocInfo s_allocInfo;

const AllocInfo& GetAllocInfo()
{
	return s_allocInfo;
}


void ArrayInit(AllocArray& arr, class IAllocator& allocator)
{
	arr.capacity = 64;
	arr.data = (void**)allocator.Allocate(arr.capacity * sizeof(void*), alignof(void*));
	arr.size = 0;
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

size_t ArrayAddOrdered(AllocArray& arr, void* elem)
{
	ASSERT2(arr.size < arr.capacity, "Not enough memory in array");

	for (size_t i = 0; i < arr.size; ++i)
	{
		if (arr.data[i] > elem)
		{
			memory::Move(arr.data + i + 1, arr.data + i, (arr.size - i) * sizeof(void*));
			arr.data[i] = elem;
			arr.size++;
			return i;
		}
	}
	arr.data[arr.size++] = elem;
	return arr.size - 1;
}

void ArrayAddOrdered(AllocArray& arr, void* elem, size_t elemIdx)
{
	ASSERT2(arr.size < arr.capacity, "Not enough memory in array");

	if (elemIdx < arr.size)
		memory::Move(arr.data + elemIdx + 1, arr.data + elemIdx, (arr.size - elemIdx) * sizeof(void*));
	arr.data[elemIdx] = elem;
	arr.size++;
}

void ArrayEraseOrdered(AllocArray& arr, void* elem)
{
	for (size_t i = 0; i < arr.size; ++i)
	{
		if (arr.data[i] == elem)
		{
			if (i < arr.size - 1)
				memory::Move(arr.data + i, arr.data + i + 1, (arr.size - i) * sizeof(void*));
			arr.size--;
			return;
		}
	}
	ASSERT2(false, "Nothing removed");
}

void ArrayEraseOrdered(AllocArray& arr, size_t elemIdx)
{
	ASSERT2(elemIdx < arr.size, "Index out of bounds");

	if (elemIdx < arr.size - 1)
		memory::Move(arr.data + elemIdx, arr.data + elemIdx + 1, (arr.size - elemIdx) * sizeof(void*));
	arr.size--;
}

bool ArrayFind(AllocArray& arr, void* elem, size_t& elemIdx)
{
	for (size_t i = 0; i < arr.size; ++i)
	{
		if (arr.data[i] == elem)
		{
			elemIdx = i;
			return true;
		}
	}
	return false;
}


static const size_t MAX_ALLOCATORS = 1000;
static HeapAllocator* s_allocatorsData[MAX_ALLOCATORS] = { nullptr };
static AllocArray s_allocators{ (void**)&s_allocatorsData, 0, MAX_ALLOCATORS };

const IAllocator** GetAllocators()
{
	return (const IAllocator**)s_allocators.data;
}

size_t GetAllocatorsSize()
{
	return s_allocators.size;
}


#endif


void* AlignPointer(void* ptr, size_t alignment)
{
	return (void*)(((uintptr)ptr + (uintptr)alignment - 1) & ~(uintptr)(alignment - 1));
}


// ---------------- MAIN ALLOCATOR ----------------


MainAllocator::MainAllocator()
{
#if DEBUG_ALLOCATORS
	ArrayAddOrdered(s_allocators, this);
#endif
}

MainAllocator::~MainAllocator()
{
#if DEBUG_ALLOCATORS
	ASSERT2(m_allocCount == 0, "Memory leak");
	ASSERT2(m_allocSize == 0, "Memory leak");

	ArrayEraseOrdered(s_allocators, this);
#endif
}

void* MainAllocator::Allocate(size_t size, size_t alignment)
{
	ScopeLock<SpinLock> lock(m_lock);

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
#else
	void* ptr = malloc(alignment + size);
	void* data = AlignPointer((char*)ptr + 1, alignment);
	*((char*)data - 1) = (char)((ptrdiff_t)data - (ptrdiff_t)ptr);
#endif

	return data;
}

void* MainAllocator::Reallocate(void* ptr, size_t size, size_t alignment)
{
	ScopeLock<SpinLock> lock(m_lock);

#if DEBUG_ALLOCATORS
	if (ptr != nullptr)
	{
		AllocHeader origHeader = *static_cast<AllocHeader*>((void*)((char*)ptr - sizeof(AllocHeader)));
		m_allocCount--;
		m_allocSize -= origHeader.allSize;
		ptr = (void*)origHeader.ptr;
	}

	size_t allocAlign = Max(alignment, alignof(AllocHeader));
	size_t allocSize = allocAlign + sizeof(AllocHeader) + alignment + size;
	void* newptr = realloc(ptr, allocSize);
	//if realloc returns memory pointer with different alignment as malloc, whole universe will explode (data pointer would points to wrong memory)
	
	if (newptr != nullptr)
	{
		void* data = AlignPointer((char*)newptr + sizeof(AllocHeader), allocAlign);
		ASSERT2((uintptr)data % alignment == 0, "Invalid alignment");
		AllocHeader* header = static_cast<AllocHeader*>((void*)((char*)data - sizeof(AllocHeader)));
		ASSERT2((uintptr)header % alignof(AllocHeader) == 0, "Invalid alignment");
		header->size = size;
		header->allSize = allocSize;
		header->ptr = newptr;

		m_allocCount++;
		m_allocSize += allocSize;

		newptr = data;
	}
#else
	if (ptr != nullptr)
	{
		void* origPtr = (char*)ptr - *((char*)ptr - 1);
		ptr = origPtr;
	}
	void* newPtr = realloc(ptr, alignment + size);
	//if realloc returns memory pointer with different alignment as malloc, whole universe will explode (data pointer would points to wrong memory)
	if (newptr != nullptr)
	{
		void* data = AlignPointer((char*)newPtr + 1, alignment);
		*((char*)data - 1) = (char)((ptrdiff_t)data - (ptrdiff_t)newPtr);
		newPtr = data;
	}
#endif

	return newptr;
}

void MainAllocator::Deallocate(void* ptr)
{
	ASSERT(ptr != nullptr);

	ScopeLock<SpinLock> lock(m_lock);

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

#if DEBUG_ALLOCATORS

i64 MainAllocator::GetAllocCount() const
{
	return m_allocCount;
}

size_t MainAllocator::GetAllocSize() const
{
	return m_allocSize;
}

#endif


// ---------------- HEAP ALLOCATOR ----------------


HeapAllocator::HeapAllocator(IAllocator& allocator, bool debug)
	: m_source(allocator)
{
#if DEBUG_ALLOCATORS
	ArrayAddOrdered(s_allocators, this);

	ArrayInit(m_allocations, m_source);
	ArrayInit(m_pages, m_source);
	ArrayInit(m_pagesCounts, m_source);
#endif
}

HeapAllocator::~HeapAllocator()
{
#if DEBUG_ALLOCATORS
	ASSERT2(m_pages.size == 0, "Memory leak");
	ASSERT2(m_pagesCounts.size == 0, "Memory leak");
	ASSERT2(m_allocations.size == 0, "Memory leak");
	ArrayDeinit(m_pages, m_source);
	ArrayDeinit(m_pagesCounts, m_source);
	ArrayDeinit(m_allocations, m_source);

	ASSERT2(m_allocCount == 0, "Memory leak");
	ASSERT2(m_allocSize == 0, "Memory leak");

	ArrayEraseOrdered(s_allocators, this);
#endif
}

void* HeapAllocator::Allocate(size_t size, size_t alignment)
{
	ScopeLock<SpinLock> lock(m_lock);

	void* data = m_source.Allocate(size, alignment);
#if DEBUG_ALLOCATORS
	m_allocCount++;
	m_allocSize += size;

	ArrayCheckSize(m_allocations, m_source);
	ArrayAddOrdered(m_allocations, data);

	void* pagePtr = (void*)(((uintptr)data / GetAllocInfo().pageSize) * GetAllocInfo().pageSize);//TODO: bit operations
	size_t arrPageIdx;
	if (ArrayFind(m_pages, pagePtr, arrPageIdx))
	{
		uintptr count = (uintptr)m_pagesCounts.data[arrPageIdx] + 1;
		m_pagesCounts.data[arrPageIdx] = (void*)count;
	}
	else
	{
		ArrayCheckSize(m_pages, m_source);
		size_t addIdx = ArrayAddOrdered(m_pages, pagePtr);
		ArrayCheckSize(m_pagesCounts, m_source);
		ArrayAddOrdered(m_pagesCounts, (void*)1, addIdx);
	}
#endif
	return data;
}

void* HeapAllocator::Reallocate(void* ptr, size_t size, size_t alignment)
{
	ScopeLock<SpinLock> lock(m_lock);

#if DEBUG_ALLOCATORS
	if (ptr != nullptr)
	{
		m_allocCount--;
		m_allocSize -= m_source.GetSize(ptr);

		ArrayEraseOrdered(m_allocations, ptr);

		void* oldPagePtr = (void*)(((uintptr)ptr / GetAllocInfo().pageSize) * GetAllocInfo().pageSize);
		size_t oldArrPageIdx;
		if (ArrayFind(m_pages, oldPagePtr, oldArrPageIdx))
		{
			uintptr count = (uintptr)m_pagesCounts.data[oldArrPageIdx] - 1;
			if (count == 0)
			{
				ArrayEraseOrdered(m_pages, oldArrPageIdx);
				ArrayEraseOrdered(m_pagesCounts, oldArrPageIdx);
			}
			else
			{
				m_pagesCounts.data[oldArrPageIdx] = (void*)count;
			}
		}
		else
		{
			ASSERT2(false, "There must be record of page for given allocation");
		}
	}
#endif

	void* data = m_source.Reallocate(ptr, size, alignment);

#if DEBUG_ALLOCATORS

	if (data != nullptr)
	{
		m_allocCount++;
		m_allocSize += size;

		ArrayCheckSize(m_allocations, m_source);
		ArrayAddOrdered(m_allocations, data);

		void* pagePtr = (void*)(((uintptr)data / GetAllocInfo().pageSize) * GetAllocInfo().pageSize);
		size_t arrPageIdx;
		if (ArrayFind(m_pages, pagePtr, arrPageIdx))
		{
			uintptr count = (uintptr)m_pagesCounts.data[arrPageIdx] + 1;
			m_pagesCounts.data[arrPageIdx] = (void*)count;
		}
		else
		{
			ArrayCheckSize(m_pages, m_source);
			size_t addIdx = ArrayAddOrdered(m_pages, pagePtr);
			ArrayCheckSize(m_pagesCounts, m_source);
			ArrayAddOrdered(m_pagesCounts, (void*)1, addIdx);
		}
	}
#endif

	return data;
}

void HeapAllocator::Deallocate(void* ptr)
{
	ASSERT(ptr != nullptr);

	ScopeLock<SpinLock> lock(m_lock);

#if DEBUG_ALLOCATORS
	m_allocCount--;
	m_allocSize -= m_source.GetSize(ptr);

	ArrayEraseOrdered(m_allocations, ptr);

	void* pagePtr = (void*)(((uintptr)ptr / GetAllocInfo().pageSize) * GetAllocInfo().pageSize);
	size_t arrPageIdx;
	if (ArrayFind(m_pages, pagePtr, arrPageIdx))
	{
		uintptr count = (uintptr)m_pagesCounts.data[arrPageIdx] - 1;
		if (count == 0)
		{
			ArrayEraseOrdered(m_pages, arrPageIdx);
			ArrayEraseOrdered(m_pagesCounts, arrPageIdx);
		}
		else
		{
			m_pagesCounts.data[arrPageIdx] = (void*)count;
		}
	}
	else
	{
		ASSERT2(false, "There must be record of page for given allocation");
	}
#endif
	m_source.Deallocate(ptr);
}

size_t HeapAllocator::GetSize(void* ptr) const
{
	return m_source.GetSize(ptr);
}


#if DEBUG_ALLOCATORS

void HeapAllocator::SetDebugName(const char* name) { m_name = name; }

const char* HeapAllocator::GetDebugName() const { return m_name; }

i64 HeapAllocator::GetAllocCount() const
{
	return m_allocCount;
}

size_t HeapAllocator::GetAllocSize() const
{
	return m_allocSize;
}

size_t HeapAllocator::GetAllocationsSize() const { return m_allocations.size; }
void** HeapAllocator::GetAllocations() const { return m_allocations.data; }
size_t HeapAllocator::GetBlocksSize() const { return m_pages.size; }
void** HeapAllocator::GetBlocks() const { return m_pages.data; }
size_t HeapAllocator::GetBlockSize() const { return GetAllocInfo().pageSize; }


#endif


}


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


void* operator new(size_t, Veng::NewPlaceholder, void* where)
{
	return where;
}


void* operator new(size_t size, Veng::IAllocator& allocator, size_t alignment)
{
	return allocator.Allocate(size, alignment);
}