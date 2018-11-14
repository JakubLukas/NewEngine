#pragma once

#include "int.h"
#include "memory.h"
#include "asserts.h"

#define DEBUG_ALLOCATORS 1


namespace Veng
{


#if DEBUG_ALLOCATORS

struct AllocArray
{
	void** data = nullptr;
	size_t size = 0;
	size_t capacity = 0;
};


struct AllocInfo
{
	void* minAddress;
	void* maxAddress;
	size_t allocationGranularity;
	size_t pageSize;
};

const AllocInfo& GetAllocInfo();

#endif

//-----------------------------------------------

void* AlignPointer(void* ptr, size_t alignment);

//-----------------------------------------------

class IAllocator
{
public:
	virtual ~IAllocator() {}

	virtual void* Allocate(size_t size, size_t alignment) = 0;
	virtual void* Reallocate(void* ptr, size_t size, size_t alignment) = 0;
	virtual void Deallocate(void* ptr) = 0;
	virtual size_t GetSize(void* ptr) const = 0;

	virtual const char* GetName() const = 0;
	virtual i64 GetAllocCount() const = 0;
	virtual size_t GetAllocSize() const = 0;
};


class MainAllocator : public IAllocator
{
public:
	MainAllocator();
	~MainAllocator() override;

	void* Allocate(size_t size, size_t alignment) override;
	void* Reallocate(void* ptr, size_t size, size_t alignment) override;
	void Deallocate(void* ptr) override;
	size_t GetSize(void* ptr) const override;

	const char* GetName() const override;
	i64 GetAllocCount() const override;
	size_t GetAllocSize() const override;

#if DEBUG_ALLOCATORS
private:
	i64 m_allocCount = 0;
	size_t m_allocSize = 0;
#endif
};


class HeapAllocator : public IAllocator
{
public:
	HeapAllocator(IAllocator& allocator, bool debug = false);
	~HeapAllocator();

	void* Allocate(size_t size, size_t alignment) override;
	void* Reallocate(void* ptr, size_t size, size_t alignment) override;
	void Deallocate(void* ptr) override;
	size_t GetSize(void* ptr) const override;

	void SetName(const char* name);
	const char* GetName() const override;
	i64 GetAllocCount() const override;
	size_t GetAllocSize() const override;

#if DEBUG_ALLOCATORS
	size_t GetAllocationsSize() const { return m_allocations.size; }
	void** GetAllocations() const { return m_allocations.data; }
	size_t GetPagesSize() const { return m_pages.size; }
	void** GetPages() const { return m_pages.data; }
#else
	size_t GetAllocationsSize() const { return 0; }
	void** GetAllocations() const { return nullptr; }
	size_t GetPagesSize() const { return 0; }
	void** GetPages() const { return nullptr; }
#endif

private:
	IAllocator& m_source;
	const char* m_name = "Heap";
#if DEBUG_ALLOCATORS
	bool m_debug = false;
	i32 m_allocCount = 0;
	size_t m_allocSize = 0;

	AllocArray m_allocations;
	AllocArray m_pages;
	AllocArray m_pagesCounts;
#endif
};


template<int maxSize>
class StackAllocator : public IAllocator
{
public:
	~StackAllocator() override {}

	void* Allocate(size_t size, size_t alignment) override
	{
		u8* ptr = (u8*)AlignPointer(m_ptr, alignment);
		ASSERT2(ptr + size <= m_memory + maxSize, "Not enough memory");
		m_ptr = ptr + size;
		return ptr;
	}

	void* Reallocate(void* ptr, size_t size, size_t alignment) override
	{
		ASSERT2(false, "Can't use reallocate");
		return ptr;
	}

	void Deallocate(void* ptr) override { }

	size_t GetSize(void* ptr) const override { return 0; }

	const char* GetName() const override { return "Stack"; }
	i64 GetAllocCount() const override { return 0; }
	size_t GetAllocSize() const override { return maxSize; }

private:
	u8 m_memory[maxSize];
	u8* m_ptr = m_memory;
};


//PoolAllocator


//FrameAllocator


//PageAllocator


//ProxyAllocator


//TraceAllocator


//TempAllocator


}


namespace Veng
{

struct NewPlaceholder { };

}

void* operator new(size_t size, Veng::NewPlaceholder, void* where);

void* operator new(size_t size, Veng::IAllocator& allocator, size_t alignment);


template<class Type>
void DeleteObject(Type* ptr)
{
	if (ptr != nullptr)
	{
		ptr->~Type();
	}
}

template<class Type>
void DeleteObject(Veng::IAllocator& allocator, Type* ptr)
{
	if (ptr != nullptr)
	{
		ptr->~Type();
		allocator.Deallocate(ptr);
	}
}

#define ALLOCATE(allocator, size, alignment)

#define NEW_OBJECT(allocator, Type) new (allocator, alignof(Type)) Type
#define DELETE_OBJECT(allocator, object) DeleteObject(allocator, object);

#define NEW_PLACEMENT(ptr, Type) new (Veng::NewPlaceholder(), static_cast<void*>(ptr)) Type
#define DELETE_PLACEMENT(ptr) DeleteObject(ptr);
