#pragma once


struct NewPlaceholder { };

void* operator new(size_t size, NewPlaceholder, void* where);


namespace Veng
{


class IAllocator
{
public:
	virtual ~IAllocator() {}

	virtual void* Allocate(size_t size) = 0;
	virtual void* Reallocate(void* ptr, size_t size) = 0;
	virtual void Deallocate(void* p) = 0;
	virtual size_t AllocatedSize(void* p) = 0;
};


class MainAllocator : public IAllocator
{
public:
	MainAllocator();
	~MainAllocator() override;

	void* Allocate(size_t size) override;
	void* Reallocate(void* ptr, size_t size) override;
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

	void* Allocate(size_t size) override;
	void* Reallocate(void* ptr, size_t size) override;
	void Deallocate(void* p) override;
	size_t AllocatedSize(void* p) override;

private:
	IAllocator& m_allocator;
};


//PoolAllocator


//FrameAllocator


//PageAllocator


//ProxyAllocator


//TraceAllocator


//TempAllocator


}