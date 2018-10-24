namespace Veng
{


template<class KeyType, class ValueType>
AssociativeArray<KeyType, ValueType>::AssociativeArray(IAllocator& allocator)
	: m_allocator(allocator)
{
}

template<class KeyType, class ValueType>
AssociativeArray<KeyType, ValueType>::AssociativeArray(AssociativeArray&& other)
	: m_allocator(other.m_allocator)
	, m_capacity(other.m_capacity)
	, m_size(other.m_size)
	, m_keys(other.m_keys)
	, m_values(other.m_values)
{
	other.m_capacity = 0;
	other.m_size = 0;
	other.m_keys = nullptr;
	other.m_values = nullptr;
}

template<class KeyType, class ValueType>
AssociativeArray<KeyType, ValueType>& AssociativeArray<KeyType, ValueType>::operator =(AssociativeArray&& other)
{
	m_allocator = other.m_allocator;
	m_capacity = other.m_capacity;
	m_size = other.m_size;
	m_keys = other.m_keys;
	m_values = other.m_values;

	other.m_capacity = 0;
	other.m_size = 0;
	other.m_keys = nullptr;
	other.m_values = nullptr;

	return *this;
}

template<class KeyType, class ValueType>
AssociativeArray<KeyType, ValueType>::~AssociativeArray()
{
	for (size_t i = 0; i < m_size; ++i)
	{
		DELETE_PLACEMENT(m_keys + i);
		DELETE_PLACEMENT(m_values + i);
	}
	if (m_keys != nullptr)
		m_allocator.Deallocate(m_keys);
}


template<class KeyType, class ValueType>
void AssociativeArray<KeyType, ValueType>::Clear()
{
	for (size_t i = 0; i < m_size; ++i)
	{
		DELETE_PLACEMENT(m_keys + i);
		DELETE_PLACEMENT(m_values + i);
	}
	m_size = 0;
}

template<class KeyType, class ValueType>
bool AssociativeArray<KeyType, ValueType>::Find(const KeyType& key, ValueType*& value) const
{
	if(m_size == 0) return false;

	size_t idx = GetIndex(key);
	//idx = (idx == 0) ? idx : idx - 1;
	if (m_keys[idx] == key)
	{
		value = &m_values[idx];
		return true;
	}

	return false;
}


template<class KeyType, class ValueType>
ValueType* AssociativeArray<KeyType, ValueType>::Begin() { return m_values; }

template<class KeyType, class ValueType>
ValueType* AssociativeArray<KeyType, ValueType>::End() { return m_values + m_size; }

template<class KeyType, class ValueType>
const ValueType* AssociativeArray<KeyType, ValueType>::Begin() const { return m_values; }

template<class KeyType, class ValueType>
const ValueType* AssociativeArray<KeyType, ValueType>::End() const { return m_values + m_size; }


template<class KeyType, class ValueType>
ValueType* AssociativeArray<KeyType, ValueType>::Insert(const KeyType& key, const ValueType& value)
{
	size_t idx = GetIndex(key);
	if (m_size == 0 || m_keys[idx] != key)
	{
		if (m_size == m_capacity) Enlarge();

		for (size_t i = m_size; i > idx; --i)
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

template<class KeyType, class ValueType>
bool AssociativeArray<KeyType, ValueType>::Erase(const KeyType& key)
{
	if (m_size == 0) return false;

	size_t idx = GetIndex(key);
	if (m_keys[idx] == key)
	{
		DELETE_PLACEMENT(m_keys + idx);
		DELETE_PLACEMENT(m_values + idx);

		for (size_t i = idx; i < m_size - 1; ++i)
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


template<class KeyType, class ValueType>
const ValueType& AssociativeArray<KeyType, ValueType>::operator[](const KeyType& key) const
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

template<class KeyType, class ValueType>
ValueType& AssociativeArray<KeyType, ValueType>::operator[](const KeyType& key)
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


template<class KeyType, class ValueType>
void AssociativeArray<KeyType, ValueType>::Reserve(size_t capacity)
{
	if (capacity <= m_capacity) return;

	m_capacity = capacity;
	void* data = m_allocator.Allocate(m_capacity * (sizeof(KeyType) + sizeof(ValueType)) + ALIGN_OF(ValueType), ALIGN_OF(KeyType));
	KeyType* newKeys = static_cast<KeyType*>(data);
	ValueType* newValues = static_cast<ValueType*>(AlignPointer(newKeys + m_capacity, ALIGN_OF(ValueType)));



	for (size_t i = 0; i < m_size; ++i)
	{
		NEW_PLACEMENT(newKeys + i, KeyType)(m_keys[i]);
		DELETE_PLACEMENT(m_keys + i);
		NEW_PLACEMENT(newValues + i, ValueType)(m_values[i]);
		DELETE_PLACEMENT(m_values + i);
	}

	if (m_keys != nullptr)
		m_allocator.Deallocate(m_keys);
	m_keys = newKeys;
	m_values = newValues;
}


template<class KeyType, class ValueType>
size_t AssociativeArray<KeyType, ValueType>::GetSize() const { return m_size; }

template<class KeyType, class ValueType>
size_t AssociativeArray<KeyType, ValueType>::GetCapacity() const { return m_capacity; }

template<class KeyType, class ValueType>
KeyType* AssociativeArray<KeyType, ValueType>::GetKeys() const { return m_keys; }

template<class KeyType, class ValueType>
ValueType* AssociativeArray<KeyType, ValueType>::GetValues() const { return m_values; }


template<class KeyType, class ValueType>
void AssociativeArray<KeyType, ValueType>::Enlarge()
{
	size_t newCapacity = (m_capacity == 0) ? INITIAL_SIZE : m_capacity * ENLARGE_MULTIPLIER;
	Reserve(newCapacity);
}

template<class KeyType, class ValueType>
size_t AssociativeArray<KeyType, ValueType>::GetIndex(const KeyType& key) const
{
	//binary search: O(log n)
	size_t low = 0;
	size_t high = m_size;
	size_t i;
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


// hack to make interface clear


template<class KeyType, class ValueType>
inline ValueType* begin(AssociativeArray<KeyType, ValueType>& a)
{
	return a.Begin();
}


template<class KeyType, class ValueType>
inline ValueType* end(AssociativeArray<KeyType, ValueType>& a)
{
	return a.End();
}


template<class KeyType, class ValueType>
inline const ValueType* begin(const AssociativeArray<KeyType, ValueType>& a)
{
	return a.Begin();
}


template<class KeyType, class ValueType>
inline const ValueType* end(const AssociativeArray<KeyType, ValueType>& a)
{
	return a.End();
}


}