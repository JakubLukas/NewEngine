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

	~AssociativeArray()
	{
		for (unsigned i = 0; i < m_size; ++i)
		{
			m_keys[i].~KeyType();
			m_values[i].~ValueType();
		}
		m_allocator.Deallocate(m_keys);
		m_allocator.Deallocate(m_values);
	}


	void Clear()
	{
		ASSERT2(false, "Not implemented yet");
		static_assert(false);
	}


	bool Find(const KeyType& key, ValueType*& value)
	{
		unsigned idx = GetIndex(key);
		if (m_keys[idx] == key)
		{
			value = &m_values[i];
			return true;
		}

		return false;
	}


	ValueType* begin() { return m_values; }
	ValueType* end() { return m_values + m_size; }

	const ValueType* begin() const { return m_values; }
	const ValueType* end() const { return m_values + m_size; }


	void Push(const KeyType& key, const ValueType& value)
	{
		if(m_size == m_capacity) Grow();

		new (NewPlaceholder, (void*)(m_keys + m_size)) KeyType(key);
		new (NewPlaceholder, (void*)(m_values + m_size)) ValueType(value);
		++m_size;
	}


	bool Insert(KeyType& key, ValueType& value)
	{
		unsigned idx = GetIndex(key);
		if (m_keys[idx] != key)
		{
			if (m_size == m_capacity) Grow();

			//<idx, m_size> move by one
			//insert on idx
			static_assert(false);
		}

		return false;
	}


	bool Erase(const KeyType& key)
	{
		ASSERT2(false, "Not implemented yet");
		static_assert(false);
	}


	const ValueType& operator[](const KeyType& key) const
	{
		ValueType* value;
		if (Find(key, value))
		{
			return *value;
		}
		else
		{
			ASSERT2(false, "Key not find");
			return ValueType();
		}
	}


	ValueType& operator[](const KeyType& key)
	{
		ValueType* value;
		if (Find(key, value))
		{
			return *value;
		}
		else
		{
			ASSERT2(false, "Key not find");
			return ValueType();
		}
	}


	void Reserve(unsigned size)
	{
		ASSERT2(false, "Not implemented yet");
		static_assert(false);
	}


	void Resize(unsigned size)
	{
		ASSERT2(false, "Not implemented yet");
		static_assert(false);
	}


	int Size() const { return m_size; }

	int Capacity() const { return m_capacity; }

private:
	unsigned GetIndex(const KeyType& key)
	{
		//binary search: O(log n)
		int low = 0;
		int high = m_size;
		int i;
		const KeyType* mkey;

		while (low < high)
		{
			i = (low + high) >> 1;
			mkey = &m_keys[i];

			if (mkey < key)
				low = i + 1;
			else if (key < mkey)
				high = i;
			else
				return i;
		}

		return (low == 0) ? low : low - 1;
	}


	void Grow()
	{
		m_capacity = m_capacity * 2;
		m_keys = static_cast<KeyType*>(m_allocator.Reallocate(m_keys, m_capacity * sizeof(KeyType)));
		m_values = static_cast<ValueType*>(m_allocator.Reallocate(m_values, m_capacity * sizeof(ValueType)));
	}


	IAllocator& m_allocator;
	unsigned m_capacity = 0;
	unsigned m_size = 0;
	KeyType* m_keys = nullptr;
	ValueType* m_values = nullptr;
};


}