#pragma once

#include "allocator.h"
#include "asserts.h"
#include "threading/threads.h"


#if DEBUG_ALLOCATORS
#	include "core/containers/array.h"
#	include "core/containers/associative_array.h"
#endif


namespace Veng
{


#if DEBUG_ALLOCATORS

struct AllocatorDebugData
{
	bool operator==(const AllocatorDebugData& other) const { return allocator == other.allocator; }
	bool operator!=(const AllocatorDebugData& other) const { return allocator != other.allocator; }
	bool operator<(const AllocatorDebugData& other) const { return allocator < other.allocator; }
	bool operator<=(const AllocatorDebugData& other) const { return allocator <= other.allocator; }
	bool operator>(const AllocatorDebugData& other) const { return allocator > other.allocator; }
	bool operator>=(const AllocatorDebugData& other) const { return allocator >= other.allocator; }

	Allocator* parent = nullptr;
	Allocator* allocator = nullptr;
};

const Array<AllocatorDebugData>& GetAllocators();

#endif

//-----------------------------------------------


class MainAllocator : public Allocator
{
public:
	MainAllocator();
	MainAllocator(MainAllocator&) = delete;
	MainAllocator(MainAllocator&&) = delete;
	MainAllocator& operator=(MainAllocator&) = delete;
	MainAllocator& operator=(MainAllocator&&) = delete;
	~MainAllocator() override;

	void* Allocate(size_t size, size_t alignment) override;
	void* Reallocate(void* ptr, size_t size, size_t alignment) override;
	void Deallocate(void* ptr) override;
	size_t GetSize(void* ptr) const override;

#if DEBUG_ALLOCATORS
	void SetDebugName(const char* name) override {}
	const char* GetDebugName() const override { return "Main"; };
	size_t GetAllocCount() const override;
	size_t GetAllocSize() const override;

	size_t GetAllocationsSize() const override { return 0; };
	AllocationDebugData const* GetAllocations() const override { return nullptr; };
	size_t GetBlocksSize() const override { return 0; }
	void* const* GetBlocks() const override { return nullptr; }
	size_t GetBlockSize() const override { return 0; }
#endif

private:
	SpinLock m_lock;
#if DEBUG_ALLOCATORS
	i64 m_allocCount = 0;
	size_t m_allocSize = 0;
#endif
};


class ProxyAllocator : public Allocator
{
public:
	ProxyAllocator(Allocator& allocator);
	ProxyAllocator(ProxyAllocator&) = delete;
	ProxyAllocator(ProxyAllocator&&) = delete;
	ProxyAllocator& operator=(ProxyAllocator&) = delete;
	ProxyAllocator& operator=(ProxyAllocator&&) = delete;
	~ProxyAllocator() override;

	void* Allocate(size_t size, size_t alignment) override;
	void* Reallocate(void* ptr, size_t size, size_t alignment) override;
	void Deallocate(void* ptr) override;
	size_t GetSize(void* ptr) const override;

#if DEBUG_ALLOCATORS
	void SetDebugName(const char* name) override;
	const char* GetDebugName() const override;
	size_t GetAllocCount() const override;
	size_t GetAllocSize() const override;

	size_t GetAllocationsSize() const override;
	AllocationDebugData const* GetAllocations() const override;
	size_t GetBlocksSize() const override;
	void* const* GetBlocks() const override;
	size_t GetBlockSize() const override;
#endif

private:
	Allocator& m_source;
	SpinLock m_lock;
#if DEBUG_ALLOCATORS
	const char* m_name = "Heap";
	i32 m_allocCount = 0;
	size_t m_allocSize = 0;

	Array<AllocationDebugData> m_allocations;
	AssociativeArray<void*, size_t> m_pages;
#endif
};


template<int maxSize>
class StackAllocator : public Allocator
{
public:
	StackAllocator();
	StackAllocator(StackAllocator&) = delete;
	StackAllocator(StackAllocator&&) = delete;
	StackAllocator& operator=(StackAllocator&) = delete;
	StackAllocator& operator=(StackAllocator&&) = delete;
	~StackAllocator() override;

	void* Allocate(size_t size, size_t alignment) override;
	void* Reallocate(void* ptr, size_t size, size_t alignment) override;
	void Deallocate(void* ptr) override;
	size_t GetSize(void* ptr) const override { return 0; }

#if DEBUG_ALLOCATORS
	void SetDebugName(const char* name) override {}
	const char* GetDebugName() const override { return "Stack"; }
	size_t GetAllocCount() const override { return 0; }
	size_t GetAllocSize() const override { return maxSize; }

	size_t GetAllocationsSize() const override { return 0; };
	AllocationDebugData const* GetAllocations() const override { return nullptr; };
	size_t GetBlocksSize() const override { return 0; }
	void* const* GetBlocks() const override { return nullptr; }
	size_t GetBlockSize() const override { return 0; }
#endif

private:
	SpinLock m_lock;
	u8 m_memory[maxSize];
	u8* m_ptr = m_memory;
};


/*class FrameAllocator : public Allocator
{
public:
	FrameAllocator(Allocator& allocator);
	FrameAllocator(FrameAllocator&) = delete;
	FrameAllocator(FrameAllocator&&) = delete;
	FrameAllocator& operator=(FrameAllocator&) = delete;
	FrameAllocator& operator=(FrameAllocator&&) = delete;
	~FrameAllocator() override;

	void* Allocate(size_t size, size_t alignment) override;
	void* Reallocate(void* ptr, size_t size, size_t alignment) override;
	void Deallocate(void* ptr) override;
	size_t GetSize(void* ptr) const override;

	void NewFrame();

#if DEBUG_ALLOCATORS
	void SetDebugName(const char* name) override;
	const char* GetDebugName() const override;
	size_t GetAllocCount() const override;
	size_t GetAllocSize() const override;

	size_t GetAllocationsSize() const override;
	AllocationDebugData const* GetAllocations() const override;
	size_t GetBlocksSize() const override;
	void* const* GetBlocks() const override;
	size_t GetBlockSize() const override;
#endif

private:
	static const size_t BLOCK_SIZE = 1'048'576;

private:
	Allocator& m_source;
	SpinLock m_lock;
	void* m_buffer;
#if DEBUG_ALLOCATORS
	const char* m_name = "Heap";
	i32 m_allocCount = 0;
	size_t m_allocSize = 0;
#endif
};*/


//-----------------------------------------------------------------------------

template<int maxSize>
StackAllocator<maxSize>::StackAllocator()
{}

template<int maxSize>
StackAllocator<maxSize>::~StackAllocator()
{}

template<int maxSize>
void* StackAllocator<maxSize>::Allocate(size_t size, size_t alignment)
{
	ScopeLock<SpinLock> lock(m_lock);

	u8* data = (u8*)AlignPointer(m_ptr, alignment);
	ASSERT2(data + size <= m_memory + maxSize, "Not enough memory");
	m_ptr = data + size;
	return data;
}

template<int maxSize>
void* StackAllocator<maxSize>::Reallocate(void* ptr, size_t size, size_t alignment)
{
	ASSERT2(false, "Can't use reallocate");
	return ptr;
}

template<int maxSize>
void StackAllocator<maxSize>::Deallocate(void* ptr)
{}



//PoolAllocator


//PageAllocator


//HeapAllocator


//TraceAllocator


//TempAllocator


}
