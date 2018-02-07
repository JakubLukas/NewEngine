#pragma once

#include "core/allocators.h"
#include "core/asserts.h"


namespace Veng
{


template<class KeyType, class ValueType>
class AssociativeArray
{
public:
	explicit AssociativeArray(IAllocator& allocator)
		: m_allocator(allocator)
	{
	}

	~AssociativeArray()
	{
		for (unsigned i = 0; i < m_size; ++i)
		{
			DELETE_PLACEMENT(m_keys + i);
			DELETE_PLACEMENT(m_values + i);
		}
		if(m_keys != nullptr)
			m_allocator.Deallocate(m_keys);
	}


	void Clear()
	{
		for(unsigned i = 0; i < m_size; ++i)
		{
			DELETE_PLACEMENT(m_keys + i);
			DELETE_PLACEMENT(m_values + i);
		}
		m_size = 0;
	}


	bool Find(const KeyType& key, ValueType*& value)
	{
		if(m_size == 0) return false;

		unsigned idx = GetIndex(key);
		//idx = (idx == 0) ? idx : idx - 1;
		if (m_keys[idx] == key)
		{
			value = &m_values[idx];
			return true;
		}

		return false;
	}


	ValueType* begin() { return m_values; }
	ValueType* end() { return m_values + m_size; }

	const ValueType* begin() const { return m_values; }
	const ValueType* end() const { return m_values + m_size; }


	ValueType* Insert(const KeyType& key, const ValueType& value)
	{
		unsigned idx = GetIndex(key);
		if (m_size == 0 || m_keys[idx] != key)
		{
			if (m_size == m_capacity) Enlarge();

			for(unsigned i = m_size; i > idx; --i)
			{
				auto& t = m_keys[i - 1];
				NEW_PLACEMENT(m_keys + i, KeyType)(t);
				DELETE_PLACEMENT(m_keys + i - 1);
				NEW_PLACEMENT(m_values + i, ValueType)(m_values[i - 1]);
				DELETE_PLACEMENT(m_values + i - 1);
			}
			
			++m_size;
			NEW_PLACEMENT(m_keys + idx, KeyType)(key);
			return NEW_PLACEMENT(m_values + idx, ValueType)(value);
		}

		return nullptr;
	}


	bool Erase(const KeyType& key)
	{
		if(m_size == 0) return false;

		unsigned idx = GetIndex(key);
		if(m_keys[idx] == key)
		{
			DELETE_PLACEMENT(m_keys + idx);
			DELETE_PLACEMENT(m_values + idx);

			for(unsigned i = idx; i < m_size - 1; ++i)
			{
				NEW_PLACEMENT(m_keys + i, KeyType)(m_keys[i + 1]);
				DELETE_PLACEMENT(m_keys + i + 1);
				NEW_PLACEMENT(m_values + i, ValueType)(m_values[i + 1]);
				DELETE_PLACEMENT(m_values + i + 1);
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
			ASSERT2(false, "Key not found");
			return m_values[0];
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
			ASSERT2(false, "Key not found");
			return m_values[0];
		}
	}


	void Reserve(unsigned capacity)
	{
		if(capacity <= m_capacity) return;

		m_capacity = capacity;
		void* data = m_allocator.Allocate(m_capacity * (sizeof(KeyType) + sizeof(ValueType)) + ALIGN_OF(ValueType), ALIGN_OF(KeyType));
		KeyType* newKeys = static_cast<KeyType*>(data);
		ValueType* newValues = static_cast<ValueType*>(AlignPointer(newKeys + m_capacity, ALIGN_OF(ValueType)));


		
		for(unsigned i = 0; i < m_size; ++i)
		{
			NEW_PLACEMENT(newKeys + i, KeyType)(m_keys[i]);
			DELETE_PLACEMENT(m_keys + i);
			NEW_PLACEMENT(newValues + i, ValueType)(m_values[i]);
			DELETE_PLACEMENT(m_values + i);
		}

		if(m_keys != nullptr)
			m_allocator.Deallocate(m_keys);
		m_keys = newKeys;
		m_values = newValues;
	}


	unsigned GetSize() const { return m_size; }

	unsigned GetCapacity() const { return m_capacity; }

private:
	void Enlarge()
	{
		unsigned newCapacity = (m_capacity == 0) ? 4 : m_capacity * 2;
		Reserve(newCapacity);
	}

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

			if (*mkey < key)
				low = i + 1;
			else if (key < *mkey)
				high = i;
			else
				return i;
		}

		return low;// (low == 0) ? low : low - 1;
	}

private:
	IAllocator& m_allocator;
	unsigned m_capacity = 0;
	unsigned m_size = 0;
	KeyType* m_keys = nullptr;
	ValueType* m_values = nullptr;
};


}