#pragma once

#include "core/allocators.h"
#include "asserts.h"
#include "utility.h"


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
		for(size_t i = 0; i < m_size; ++i)
		{
			DELETE_PLACEMENT(m_data + i);
		}
		if(m_data != nullptr)
			m_allocator.Deallocate(m_data);
	}


	void Clear()
	{
		for(size_t i = 0; i < m_size; ++i)
		{
			DELETE_PLACEMENT(m_data + i);
		}
		m_size = 0;
	}


	Type* Begin() { return m_data; }
	Type* End() { return m_data + m_size; }

	const Type* Begin() const { return m_data; }
	const Type* End() const { return m_data + m_size; }


	Type& PushBack(const Type& value)
	{
		if(m_size == m_capacity)
			Enlarge();

		NEW_PLACEMENT(m_data + m_size, Type)(value);

		return m_data[m_size++];
	}

	Type& PushBack()
	{
		if (m_size == m_capacity)
			Enlarge();

		NEW_PLACEMENT(m_data + m_size, Type)();

		return m_data[m_size++];
	}

	template< class... Args >
	Type& EmplaceBack(Args&&... args)
	{
		if(m_size == m_capacity)
			Enlarge();

		NEW_PLACEMENT(m_data + m_size, Type)(Utils::Forward<Args>(args)...);

		return m_data[m_size++];
	}

	Type PopBack()
	{
		ASSERT(m_size > 0);
		--m_size;
		Type result(m_data[m_size]);
		DELETE_PLACEMENT(m_data + m_size);
		return result;
	}

	void Erase(size_t idx)
	{
		ASSERT(idx < m_size);
		DELETE_PLACEMENT(m_data + idx);

		m_size--;

		NEW_PLACEMENT(m_data + idx, Type)(m_data[m_size]);
		DELETE_PLACEMENT(m_data + m_size);
	}

	const Type& operator[](size_t index) const
	{
		ASSERT(index < m_size);
		return m_data[index];
	}


	Type& operator[](size_t index)
	{
		ASSERT(index < m_size);
		return m_data[index];
	}


	void Reserve(size_t capacity)
	{
		if(capacity <= m_capacity) return;

		m_capacity = capacity;
		Type* newData = static_cast<Type*>(m_allocator.Allocate(m_capacity * sizeof(Type), ALIGN_OF(Type)));

		for(size_t i = 0; i < m_size; ++i)
		{
			NEW_PLACEMENT(newData + i, Type)(m_data[i]);
			DELETE_PLACEMENT(m_data + i);
		}

		if(m_data != nullptr)
			m_allocator.Deallocate(m_data);
		m_data = newData;
	}


	void Resize(size_t size)
	{
		if (size == m_size && size == m_capacity) return;

		Type* newData = static_cast<Type*>(m_allocator.Allocate(size * sizeof(Type), ALIGN_OF(Type)));

		size_t sizeMin = (size < m_size) ? size : m_size;
		for (size_t i = 0; i < sizeMin; ++i)
		{
			NEW_PLACEMENT(newData + i, Type)(m_data[i]);
		}
		for (size_t i = 0; i < m_size; ++i)
		{
			DELETE_PLACEMENT(m_data + i);
		}

		if(m_data != nullptr)
			m_allocator.Deallocate(m_data);
		m_data = newData;
		m_capacity = m_size = size;
	}


	size_t GetSize() const { return m_size; }

	size_t GetCapacity() const { return m_capacity; }

private:
	void Enlarge()
	{
		size_t newCapacity = (m_capacity == 0) ? 4 : m_capacity * 2;
		Reserve(newCapacity);
	}

private:
	IAllocator& m_allocator;
	size_t m_capacity = 0;
	size_t m_size = 0;
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