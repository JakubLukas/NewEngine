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
		void* data = m_allocator.Allocate(m_capacity * (sizeof(KeyType) + sizeof(ValueType)));
		m_keys = static_cast<KeyType*>(data);
		m_values = static_cast<ValueType*>(data + sizeof(KeyType) * m_capacity);
	}

	~AssociativeArray()
	{
		for (unsigned i = 0; i < m_size; ++i)
		{
			m_keys[i].~KeyType();
			m_values[i].~ValueType();
		}
		m_allocator.Deallocate(m_keys);
	}


	void Clear()
	{
		for(unsigned i = 0; i < m_size; ++i)
		{
			m_keys[i].~KeyType();
			m_values[i].~ValueType();
		}
		m_size = 0;
	}


	bool Find(const KeyType& key, ValueType*& value)
	{
		if(m_size == 0) return false;

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


	bool Insert(const KeyType& key, const ValueType& value)
	{
		unsigned idx = GetIndex(key);
		if (m_size == 0 || m_keys[idx] != key)
		{
			if (m_size == m_capacity) Reserve(m_capacity * 2);

			for(unsigned i = m_size; i > idx; --i)
			{
				m_keys[i] = m_keys[i - 1];
				m_keys[i].~KeyType();
				m_values[i] = m_values[i - 1];
				m_values[i].~ValueType();
			}
			
			m_keys[idx] = key;
			m_values[idx] = value;
			++m_size;
			return true;
		}

		return false;
	}


	bool Erase(const KeyType& key)
	{
		if(m_size == 0) return false;

		unsigned idx = GetIndex(key);
		if(m_keys[idx] == key)
		{
			m_keys[idx].~KeyType();
			m_values[idx].~ValueType();

			for(unsigned i = idx; i < m_size - 1; ++i)
			{
				m_keys[i] = m_keys[i + 1];
				m_keys[i + 1].~KeyType();
				m_values[i] = m_values[i + 1];
				m_values[i + 1].~ValueType();
			}
			--m_size;
			return true;
		}

		return false;
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


	void Reserve(unsigned capacity)
	{
		if(capacity <= m_capacity) return;

		m_capacity = capacity;
		void* data = m_allocator.Allocate(m_capacity * (sizeof(KeyType) + sizeof(ValueType)));
		KeyType* newKeys = static_cast<KeyType*>(data);
		ValueType* newValues = static_cast<ValueType*>(data + sizeof(KeyType) * m_capacity);
		
		for(unsigned i = 0; i < m_size; ++i)
		{
			newKeys[i] = m_keys[i];
			m_keys[i].~KeyType();
			newValues[i] = m_values[i];
			m_values[i].~ValueType();
		}

		m_allocator.Deallocate(m_keys);
		m_keys = newKeys;
		m_values = newValues;
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

	IAllocator& m_allocator;
	unsigned m_capacity = 0;
	unsigned m_size = 0;
	KeyType* m_keys = nullptr;
	ValueType* m_values = nullptr;
};


}