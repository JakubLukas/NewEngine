#pragma once

#include "core/allocators.h"


namespace Veng
{


template<class Type>
class Array
{
public:
	Array(IAllocator& allocator)
		: m_allocator(allocator)
	{
		m_capacity = 4;
		m_data = static_cast<Type*>(m_allocator.Allocate(m_capacity * sizeof(Type)));
	}


	void Push(const Type& value)
	{
		if(m_size == m_capacity)
			Grow();

		new (NewPlaceholder, (void*)(m_data + m_size)) Type(value);
		++m_size;
	}


	const Type& operator[](int index) const
	{
		ASSERT(index >= 0 && index < m_size);
		return m_data[index];
	}

	//Clear
	//Pop
	//Resize
	//Erase

	Type& operator[](int index)
	{
		ASSERT(index >= 0 && index < m_size);
		return m_data[index];
	}


	int Size() const { return m_size; }

	int Capacity() const { return m_capacity; }

private:
	void Grow()
	{
		m_capacity = m_capacity * 2;
		m_data = static_cast<Type*>(m_allocator.Reallocate(m_data, m_capacity * sizeof(Type)));
	}


	IAllocator& m_allocator;
	int m_capacity = 0;
	int m_size = 0;
	Type* m_data = nullptr;
};


}