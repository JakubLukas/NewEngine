#pragma once

#include "iallocator.h"


namespace Veng
{


/*class DebugAllocator : public IAllocator
{
public:
	DebugAllocator(IAllocator& allocator);
	~DebugAllocator();

	void* Allocate(size_t size, size_t alignment) override;
	void* Reallocate(void* ptr, size_t size, size_t alignment) override;
	void Deallocate(void* ptr) override;
	size_t GetSize(void* ptr) const override;

private:
	IAllocator& m_source;
	i32 m_allocCount;
};*/


}