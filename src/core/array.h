#pragma once

#include "core/allocators.h"
#include "asserts.h"


namespace Veng
{


template<class Type>
class Array
{
public:
	explicit Array(IAllocator& allocator)
		: m_allocator(allocator)
	{
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


	Type* Begin() { return m_data; }
	Type* End() { return m_data + m_size; }

	const Type* Begin() const { return m_data; }
	const Type* End() const { return m_data + m_size; }


	void Push(const Type& value)
	{
		if(m_size == m_capacity)
			Enlarge();

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
		Type* newData = static_cast<Type*>(m_allocator.Allocate(m_capacity * sizeof(Type), ALIGN_OF(Type)));

		for(unsigned i = 0; i < m_size; ++i)
		{
			new (NewPlaceholder(), (void*)(newData + i)) Type(m_data[i]);
			(m_data + i)->~Type();
		}

		m_allocator.Deallocate(m_data);
		m_data = newData;
	}


	void Resize(unsigned size)
	{
		if (size == m_size && size == m_capacity) return;

		Type* newData = static_cast<Type*>(m_allocator.Allocate(size * sizeof(Type), ALIGN_OF(Type)));

		unsigned sizeMin = (size < m_size) ? size : m_size;
		for (unsigned i = 0; i < sizeMin; ++i)
		{
			new (NewPlaceholder(), (void*)(newData + i)) Type(m_data[i]);
		}
		for (unsigned i = 0; i < m_size; ++i)
		{
			(m_data + i)->~Type();
		}

		m_allocator.Deallocate(m_data);
		m_data = newData;
		m_capacity = m_size = size;
	}


	unsigned Size() const { return m_size; }

	unsigned Capacity() const { return m_capacity; }

private:
	void Enlarge()
	{
		unsigned newCapacity = (m_capacity == 0) ? 4 : m_capacity * 2;
		Reserve(newCapacity);
	}

	IAllocator& m_allocator;
	unsigned m_capacity = 0;
	unsigned m_size = 0;
	Type* m_data = nullptr;
};


template<class Type>
Type* begin(Array<Type>& a)
{
	return a.Begin();
}


template<class Type>
Type* end(Array<Type>& a)
{
	return a.End();
}


template<class Type>
const Type* begin(const Array<Type>& a)
{
	return a.Begin();
}


template<class Type>
const Type* end(const Array<Type>& a)
{
	return a.End();
}


}