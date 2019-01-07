#pragma once

#include "core/int.h"
#include "core/iallocator.h"
#include "core/asserts.h"
#include "core/utility.h"


#define DEBUG_HANDLE_ARRAY 1


namespace Veng
{


template<class Type, class UnusedType = i32>
class HandleArray final
{
public:
	explicit HandleArray(IAllocator& allocator);
	HandleArray(HandleArray&) = delete;
	HandleArray(HandleArray&& other);
	HandleArray& operator =(const HandleArray&) = delete;
	HandleArray& operator =(const HandleArray&& other);
	~HandleArray();

	size_t Add(const Type& value);
	void Remove(size_t handle);

	size_t GetSize() const;
	size_t GetCapacity() const;

private:
	union DataType
	{
		Type data;
		UnusedType next;
#		if DEBUG_HANDLE_ARRAY
		bool alive;
#		endif
	};

private:
	template<class... Args>
	Type& EmplaceBack(Args&&... args);
	void Enlarge();

private:
	IAllocator& m_allocator;
	size_t m_capacity = 0;
	size_t m_size = 0;
	DataType* m_data = nullptr;
	UnusedType m_unused = -1;
	size_t m_unusedCount = 0;
};


}


#include "internal/handle_array.inl"
