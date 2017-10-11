#pragma once

#include "core/allocators.h"
#include "asserts.h"


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


	~Array()
	{
		for(unsigned i = 0; i < m_size; ++i)
		{
			m_data[i].~Type();
		}
		m_allocator.Deallocate(m_data);
	}


	void Clear()
	{
		for(unsigned i = 0; i < m_size; ++i)
		{
			m_data[i].~Type();
		}
		m_size = 0;
	}


	Type* begin() { return m_data; }
	Type* end() { return m_data + m_size; }

	const Type* begin() const { return m_data; }
	const Type* end() const { return m_data + m_size; }


	void Push(const Type& value)
	{
		if(m_size == m_capacity)
			Reserve(m_capacity * 2);

		new (NewPlaceholder(), (void*)(m_data + m_size)) Type(value);
		++m_size;
	}

	void Pop()
	{
		(m_data + m_size)->~Type();
		--m_size;
	}

	void Erase(unsigned idx)
	{
		ASSERT(idx >= 0 && idx < m_size);
		(m_data + idx)->~Type();

		for(unsigned i = idx; i < m_size - 1; ++i)
		{
			new (NewPlaceholder(), (void*)(m_data + i)) Type(m_data[i + 1]);
			(m_data + i + 1)->~Type();
		}
		--m_size;
	}

	const Type& operator[](unsigned index) const
	{
		ASSERT(index >= 0 && index < m_size);
		return m_data[index];
	}


	Type& operator[](unsigned index)
	{
		ASSERT(index >= 0 && index < m_size);
		return m_data[index];
	}


	void Reserve(unsigned capacity)
	{
		if(capacity <= m_capacity) return;

		m_capacity = capacity;
		Type* newData = static_cast<Type*>(m_allocator.Allocate(m_capacity * sizeof(Type)));

		for(unsigned i = 0; i < m_size; ++i)
		{
			new (NewPlaceholder(), (void*)(newData + i)) Type(m_data[i]);
			(m_data + i)->~Type();
		}

		m_allocator.Deallocate(m_data);
		m_data = newData;
	}


	unsigned Size() const { return m_size; }

	unsigned Capacity() const { return m_capacity; }

private:

	IAllocator& m_allocator;
	unsigned m_capacity = 0;
	unsigned m_size = 0;
	Type* m_data = nullptr;
};


}