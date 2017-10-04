#pragma once

#include "core/allocators.h"
#include "core/asserts.h"


namespace Veng
{


template<class KeyType, class ValueType>
class AssociativeArray
{
public:
	AssociativeArray(IAllocator& allocator)
		: m_allocator(allocator)
	{
		m_capacity = 4;
		m_keys = static_cast<KeyType*>(m_allocator.Allocate(m_capacity * sizeof(KeyType)));
		m_values = static_cast<ValueType*>(m_allocator.Allocate(m_capacity * sizeof(ValueType)));
	}


	void Push(const KeyType& key, const ValueType& value)
	{
		if(m_size == m_capacity)
			Grow();

		new (NewPlaceholder, (void*)(m_keys + m_size)) KeyType(key);
		new (NewPlaceholder, (void*)(m_values + m_size)) ValueType(value);
		++m_size;
	}


	const ValueType& operator[](const KeyType& key) const
	{
		for(unsigned i = 0; i < m_size; ++i)
		{
			if(m_keys[i] == key)
				return m_values[i];
		}

		ASSERT2(false, "Key not find");
		return m_values[0];
	}

	//Clear
	//Pop
	//Resize
	//Erase

	ValueType& operator[](const KeyType& key)
	{
		for(unsigned i = 0; i < m_size; ++i)
		{
			if(m_keys[i] == key)
				return m_values[i];
		}

		Push(key, ValueType());
		return m_data[m_size - 1];
	}


	int Size() const { return m_size; }

	int Capacity() const { return m_capacity; }

private:
	void Grow()
	{
		m_capacity = m_capacity * 2;
		m_keys = static_cast<KeyType*>(m_allocator.Reallocate(m_keys, m_capacity * sizeof(KeyType)));
		m_values = static_cast<ValueType*>(m_allocator.Reallocate(m_values, m_capacity * sizeof(ValueType)));
	}


	IAllocator& m_allocator;
	int m_capacity = 0;
	int m_size = 0;
	KeyType* m_keys = nullptr;
	ValueType* m_values = nullptr;
};


}