#pragma once

#include "int.h"
#include "memory.h"
#include "asserts.h"


namespace Veng
{


#define ALIGN_OF(x) __alignof(x)

void* AlignPointer(void* ptr, size_t alignment);


class IAllocator
{
public:
	virtual ~IAllocator() {}

	virtual void* Allocate(size_t size, size_t alignment) = 0;
	virtual void* Reallocate(void* ptr, size_t size, size_t alignment) = 0;
	virtual void Deallocate(void* p) = 0;
	virtual size_t AllocatedSize(void* p) = 0;
};


class MainAllocator : public IAllocator
{
public:
	MainAllocator();
	~MainAllocator() override;

	void* Allocate(size_t size, size_t alignment) override;
	void* Reallocate(void* ptr, size_t size, size_t alignment) override;
	void Deallocate(void* p) override;
	size_t AllocatedSize(void* p) override;

private:
	i32 m_allocCount;
};


class HeapAllocator : public IAllocator
{
public:
	HeapAllocator(IAllocator& allocator);
	~HeapAllocator();

	void* Allocate(size_t size, size_t alignment) override;
	void* Reallocate(void* ptr, size_t size, size_t alignment) override;
	void Deallocate(void* p) override;
	size_t AllocatedSize(void* p) override;

private:
	IAllocator& m_source;
	i32 m_allocCount;
};


template<int Size>
class StackAllocator : public IAllocator
{
public:
	~StackAllocator() override {}

	void* Allocate(size_t size, size_t alignment) override
	{
		u8* ptr = (u8*)AlignPointer(m_ptr, alignment);
		m_ptr = ptr + size;
		return ptr;
	}

	void* Reallocate(void* ptr, size_t size, size_t alignment) override
	{
		ASSERT2(false, "Can't use reallocate");
		return ptr;
	}

	void Deallocate(void* p) override { }

	size_t AllocatedSize(void* p) override { return 0; }

private:
	u8 m_memory[Size];
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

#define NEW_OBJECT(allocator, Type) new (allocator, ALIGN_OF(Type)) Type
#define DELETE_OBJECT(allocator, object) DeleteObject(allocator, object);

#define NEW_PLACEMENT(ptr, Type) new (Veng::NewPlaceholder(), static_cast<void*>(ptr)) Type
#define DELETE_PLACEMENT(ptr) DeleteObject(ptr);
