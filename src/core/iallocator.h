#pragma once

#include "int.h"

#define DEBUG_ALLOCATORS 1


namespace Veng
{


#if DEBUG_ALLOCATORS

struct AllocationDebugData
{
	static const size_t CALLSTACK_SIZE = 16;

	bool operator==(const AllocationDebugData& other) const { return allocation == other.allocation; }
	bool operator!=(const AllocationDebugData& other) const { return allocation != other.allocation; }
	bool operator<(const AllocationDebugData& other) const { return allocation < other.allocation; }
	bool operator<=(const AllocationDebugData& other) const { return allocation <= other.allocation; }
	bool operator>(const AllocationDebugData& other) const { return allocation > other.allocation; }
	bool operator>=(const AllocationDebugData& other) const { return allocation >= other.allocation; }

	void* allocation = nullptr;
	void* callstack[CALLSTACK_SIZE] = { 0 };
};

#endif


void* AlignPointer(void* ptr, size_t alignment);


class IAllocator
{
public:
	virtual ~IAllocator() {}

	virtual void* Allocate(size_t size, size_t alignment) = 0;
	virtual void* Reallocate(void* ptr, size_t size, size_t alignment) = 0;
	virtual void Deallocate(void* ptr) = 0;
	virtual size_t GetSize(void* ptr) const = 0;

	#if DEBUG_ALLOCATORS
	virtual void SetDebugName(const char* name) = 0;
	virtual const char* GetDebugName() const = 0;
	virtual size_t GetAllocCount() const = 0;
	virtual size_t GetAllocSize() const = 0;

	virtual size_t GetAllocationsSize() const = 0;
	virtual AllocationDebugData const* GetAllocations() const = 0;
	virtual size_t GetBlocksSize() const = 0;
	virtual void* const* GetBlocks() const = 0;
	virtual size_t GetBlockSize() const = 0;
	#else
	void SetDebugName(const char* name) {}
	const char* GetDebugName() const { return ""; }
	size_t GetAllocCount() const { return 0; }
	size_t GetAllocSize() const { return 0; }

	size_t GetAllocationsSize() const { return 0; }
	AllocationDebugData const* GetAllocations() const { return nullptr; }
	size_t GetBlocksSize() const { return 0; }
	void* const* GetBlocks() const { return nullptr; }
	size_t GetBlockSize() const { return 0; }
	#endif
};


struct NewPlaceholder {};

}


void* operator new(size_t size, Veng::NewPlaceholder, void* where);

void* operator new(size_t size, Veng::IAllocator& allocator, size_t alignment);


template<class Type>
void DeleteObject(Type* ptr)
{
	if(ptr != nullptr)
	{
		ptr->~Type();
	}
}

template<class Type>
void DeleteObject(Veng::IAllocator& allocator, Type* ptr)
{
	if(ptr != nullptr)
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
