#pragma once

#include "allocator.h"


namespace Veng
{


/*class DebugAllocator : public Allocator
{
public:
	DebugAllocator(Allocator& allocator);
	~DebugAllocator();

	void* Allocate(size_t size, size_t alignment) override;
	void* Reallocate(void* ptr, size_t size, size_t alignment) override;
	void Deallocate(void* ptr) override;
	size_t GetSize(void* ptr) const override;

private:
	Allocator& m_source;
	i32 m_allocCount;
};*/


}