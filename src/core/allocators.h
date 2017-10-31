#pragma once

#include "memory.h"


namespace Veng
{


#define ALIGN_OF(x) __alignof(x)


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
	unsigned m_allocCount;
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
};


template<class Type, class... Params>
Type* New(IAllocator& allocator, Params&&... args)
{
	return new (NewPlaceholder(), allocator) Type(forward<Params>(args)...);
}


//PoolAllocator


//FrameAllocator


//PageAllocator


//ProxyAllocator


//TraceAllocator


//TempAllocator


}

struct NewPlaceholder { };

void* operator new(size_t size, NewPlaceholder, void* where);

void* operator new(size_t size, Veng::IAllocator& allocator, size_t alignment);
