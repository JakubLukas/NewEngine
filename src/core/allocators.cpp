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

static const size_t MAX_ALLOCATORS = 1000;
static StackAllocator<(1000 + 1) * sizeof(AllocatorDebugData)> s_allocatorsAllocator;
static Array<AllocatorDebugData> s_allocators(s_allocatorsAllocator);

const Array<AllocatorDebugData>& GetAllocators()
{
	return s_allocators;
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
	s_allocators.Reserve(MAX_ALLOCATORS);
	AllocatorDebugData data;
	data.allocator = this;
	s_allocators.PushBack(data);
#endif
}

MainAllocator::~MainAllocator()
{
#if DEBUG_ALLOCATORS
	ASSERT2(m_allocCount == 0, "Memory leak");
	ASSERT2(m_allocSize == 0, "Memory leak");

	AllocatorDebugData data;
	data.allocator = this;
	s_allocators.Erase(data);
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
	void* newPtr = realloc(ptr, allocSize);
	//if realloc returns memory pointer with different alignment as malloc, whole universe will explode (data pointer would points to wrong memory)

	if (newPtr != nullptr)
	{
		void* data = AlignPointer((char*)newPtr + sizeof(AllocHeader), allocAlign);
		ASSERT2((uintptr)data % alignment == 0, "Invalid alignment");
		AllocHeader* header = static_cast<AllocHeader*>((void*)((char*)data - sizeof(AllocHeader)));
		ASSERT2((uintptr)header % alignof(AllocHeader) == 0, "Invalid alignment");
		header->size = size;
		header->allSize = allocSize;
		header->ptr = newPtr;

		m_allocCount++;
		m_allocSize += allocSize;

		newPtr = data;
	}
#else
	if (ptr != nullptr)
	{
		void* origPtr = (char*)ptr - *((char*)ptr - 1);
		ptr = origPtr;
	}
	void* newPtr = realloc(ptr, alignment + size);
	//if realloc returns memory pointer with different alignment as malloc, whole universe will explode (data pointer would points to wrong memory)
	if (newPtr != nullptr)
	{
		void* data = AlignPointer((char*)newPtr + 1, alignment);
		*((char*)data - 1) = (char)((ptrdiff_t)data - (ptrdiff_t)newPtr);
		newPtr = data;
	}
#endif

	return newPtr;
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

size_t MainAllocator::GetAllocCount() const
{
	return m_allocCount;
}

size_t MainAllocator::GetAllocSize() const
{
	return m_allocSize;
}

#endif


// ---------------- PROXY ALLOCATOR ----------------


ProxyAllocator::ProxyAllocator(Allocator& allocator)
	: m_source(allocator)
#if DEBUG_ALLOCATORS
	, m_allocations(allocator)
	, m_pages(allocator)
#endif
{
#if DEBUG_ALLOCATORS
	AllocatorDebugData data;
	data.parent = &allocator;
	data.allocator = this;
	s_allocators.PushBack(data);
#endif
}

ProxyAllocator::~ProxyAllocator()
{
#if DEBUG_ALLOCATORS
	ASSERT2(m_pages.GetSize() == 0, "Memory leak");
	ASSERT2(m_allocations.GetSize() == 0, "Memory leak");

	ASSERT2(m_allocCount == 0, "Memory leak");
	ASSERT2(m_allocSize == 0, "Memory leak");

	AllocatorDebugData data;
	data.allocator = this;
	s_allocators.Erase(data);
#endif
}

void* ProxyAllocator::Allocate(size_t size, size_t alignment)
{
	ScopeLock<SpinLock> lock(m_lock);

	void* data = m_source.Allocate(size, alignment);
#if DEBUG_ALLOCATORS
	m_allocCount++;
	m_allocSize += size;

	AllocationDebugData allocData;
	allocData.allocation = data;
	os::GetCallStack(0, 16, allocData.callstack);
	m_allocations.AddOrdered(allocData);

	void* pagePtr = (void*)(((uintptr)data / GetAllocInfo().pageSize) * GetAllocInfo().pageSize);//TODO: bit operations
	size_t* count;
	if (m_pages.Find(pagePtr, count))
	{
		(*count)++;
	}
	else
	{
		m_pages.Insert(pagePtr, 1);
	}
#endif
	return data;
}

void* ProxyAllocator::Reallocate(void* ptr, size_t size, size_t alignment)
{
	ScopeLock<SpinLock> lock(m_lock);

#if DEBUG_ALLOCATORS
	if (ptr != nullptr)
	{
		m_allocCount--;
		m_allocSize -= m_source.GetSize(ptr);

		AllocationDebugData allocData;
		allocData.allocation = ptr;
		size_t idx;
		ASSERT(m_allocations.Find(allocData, idx));
		allocData = m_allocations[idx];

		m_allocations.EraseOrdered(idx);

		void* oldPagePtr = (void*)(((uintptr)ptr / GetAllocInfo().pageSize) * GetAllocInfo().pageSize);
		size_t* oldCount;
		if (m_pages.Find(oldPagePtr, oldCount))
		{
			(*oldCount)--;
			if (*oldCount == 0)
				m_pages.Erase(oldPagePtr);
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

		AllocationDebugData allocData;
		allocData.allocation = data;
		m_allocations.AddOrdered(allocData);

		void* pagePtr = (void*)(((uintptr)data / GetAllocInfo().pageSize) * GetAllocInfo().pageSize);
		size_t* count;
		if (m_pages.Find(pagePtr, count))
		{
			(*count)++;
		}
		else
		{
			m_pages.Insert(pagePtr, 1);
		}
	}
#endif

	return data;
}

void ProxyAllocator::Deallocate(void* ptr)
{
	ASSERT(ptr != nullptr);

	ScopeLock<SpinLock> lock(m_lock);

#if DEBUG_ALLOCATORS
	m_allocCount--;
	m_allocSize -= m_source.GetSize(ptr);

	AllocationDebugData allocData;
	allocData.allocation = ptr;
	size_t idx;
	ASSERT(m_allocations.Find(allocData, idx));
	allocData = m_allocations[idx];
#endif

	m_source.Deallocate(ptr);

#if DEBUG_ALLOCATORS
	m_allocations.EraseOrdered(idx);

	void* pagePtr = (void*)(((uintptr)ptr / GetAllocInfo().pageSize) * GetAllocInfo().pageSize);
	size_t* count;
	if (m_pages.Find(pagePtr, count))
	{
		(*count)--;
		if (*count == 0)
			m_pages.Erase(pagePtr);
	}
	else
	{
		ASSERT2(false, "There must be record of page for given allocation");
	}
#endif
}

size_t ProxyAllocator::GetSize(void* ptr) const
{
	return m_source.GetSize(ptr);
}


#if DEBUG_ALLOCATORS

void ProxyAllocator::SetDebugName(const char* name) { m_name = name; }
const char* ProxyAllocator::GetDebugName() const { return m_name; }

size_t ProxyAllocator::GetAllocCount() const { return m_allocCount; }
size_t ProxyAllocator::GetAllocSize() const { return m_allocSize; }

size_t ProxyAllocator::GetAllocationsSize() const { return m_allocations.GetSize(); }
AllocationDebugData const* ProxyAllocator::GetAllocations() const { return m_allocations.Begin(); }
size_t ProxyAllocator::GetBlocksSize() const { return m_pages.GetSize(); }
void* const* ProxyAllocator::GetBlocks() const { return m_pages.GetKeys(); }
size_t ProxyAllocator::GetBlockSize() const { return GetAllocInfo().pageSize; }


#endif


// ---------------- FRAME ALLOCATOR ----------------


/*FrameAllocator::FrameAllocator(Allocator& allocator)
	: m_source(allocator)
{
#if DEBUG_ALLOCATORS
	AllocatorDebugData data;
	data.parent = &allocator;
	data.allocator = this;
	s_allocators.PushBack(data);
#endif
}

FrameAllocator::~FrameAllocator()
{
#if DEBUG_ALLOCATORS
	ASSERT2(m_allocCount == 0, "Memory leak");
	ASSERT2(m_allocSize == 0, "Memory leak");

	AllocatorDebugData data;
	data.allocator = this;
	s_allocators.Erase(data);
#endif
}


void* FrameAllocator::Allocate(size_t size, size_t alignment)
{}

void* FrameAllocator::Reallocate(void* ptr, size_t size, size_t alignment)
{}

void FrameAllocator::Deallocate(void* ptr)
{}

size_t FrameAllocator::GetSize(void* ptr) const
{}

void FrameAllocator::NewFrame()
{}


#if DEBUG_ALLOCATORS

void FrameAllocator::SetDebugName(const char* name)
{}

const char* FrameAllocator::GetDebugName() const
{}

size_t FrameAllocator::GetAllocCount() const
{}

size_t FrameAllocator::GetAllocSize() const
{}


size_t FrameAllocator::GetAllocationsSize() const
{}

AllocationDebugData const* FrameAllocator::GetAllocations() const
{}

size_t FrameAllocator::GetBlocksSize() const
{}

void* const* FrameAllocator::GetBlocks() const
{}

size_t FrameAllocator::GetBlockSize() const
{}

#endif*/


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


void* operator new(size_t size, Veng::Allocator& allocator, size_t alignment)
{
	return allocator.Allocate(size, alignment);
}