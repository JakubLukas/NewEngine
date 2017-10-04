#pragma once


enum NewPlaceholder
{

};

void* operator new(size_t size, NewPlaceholder, void* where);


namespace Veng
{


class IAllocator
{
public:
	virtual void* Allocate(size_t size) = 0;
	virtual void* Reallocate(void* ptr, size_t size) = 0;
	virtual void Deallocate(void* p) = 0;
	virtual size_t AllocatedSize(void* p) = 0;
};


class HeapAllocator : public IAllocator
{
	void* Allocate(size_t size) override;
	void* Reallocate(void* ptr, size_t size) override;
	void Deallocate(void* p) override;
	size_t AllocatedSize(void* p) override;
};


//PoolAllocator


//FrameAllocator


//PageAllocator


//ProxyAllocator


//TraceAllocator


//TempAllocator


}