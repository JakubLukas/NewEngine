namespace Veng
{


template<class Type>
Array<Type>::Array(Allocator& allocator)
	: m_allocator(allocator)
{
}

template<class Type>
Array<Type>::Array(Array&& other)
	: m_allocator(other.m_allocator)
	, m_capacity(other.m_capacity)
	, m_size(other.m_size)
	, m_data(other.m_data)
{
	other.m_capacity = 0;
	other.m_size = 0;
	other.m_data = nullptr;
}

template<class Type>
Array<Type>& Array<Type>::operator =(Array&& other)
{
	size_t capacity = m_capacity;
	size_t size = m_size;
	Type* data = m_data;

	m_allocator = other.m_allocator;

	m_capacity = other.m_capacity;
	m_size = other.m_size;
	m_data = other.m_data;

	other.m_capacity = capacity;
	other.m_size = size;
	other.m_data = data;

	return *this;
}

template<class Type>
Array<Type>::~Array()
{
	for (size_t i = 0; i < m_size; ++i)
	{
		DELETE_PLACEMENT(m_data + i);
	}
	if (m_data != nullptr)
		m_allocator.Deallocate(m_data);
}

template<class Type>
void Array<Type>::Clear()
{
	for (size_t i = 0; i < m_size; ++i)
	{
		DELETE_PLACEMENT(m_data + i);
	}
	m_size = 0;
}

template<class Type>
Type* Array<Type>::Begin() { return m_data; }

template<class Type>
Type* Array<Type>::End() { return m_data + m_size; }

template<class Type>
const Type* Array<Type>::Begin() const { return m_data; }

template<class Type>
const Type* Array<Type>::End() const { return m_data + m_size; }


template<class Type>
Type& Array<Type>::PushBack()
{
	if (m_size == m_capacity)
		Enlarge();

	NEW_PLACEMENT(m_data + m_size, Type)();

	return m_data[m_size++];
}

template<class Type>
Type& Array<Type>::PushBack(const Type& value)
{
	if (m_size == m_capacity)
		Enlarge();

	NEW_PLACEMENT(m_data + m_size, Type)(value);

	return m_data[m_size++];
}

template<class Type>
Type& Array<Type>::PushBack(Type&& value)
{
	if (m_size == m_capacity)
		Enlarge();

	NEW_PLACEMENT(m_data + m_size, Type)(Utils::Forward<Type>(value));

	return m_data[m_size++];
}

template<class Type>
Type& Array<Type>::AddOrdered(const Type& value)
{
	if (m_size == m_capacity)
		Enlarge();

	for (size_t i = 0; i < m_size; ++i)
	{
		if (m_data[i] > value)
		{
			memory::Move(m_data + i + 1, m_data + i, (m_size - i) * sizeof(Type));
			m_data[i] = value;
			m_size++;
			return m_data[i];
		}
	}
	m_data[m_size++] = value;
	return m_data[m_size - 1];
}


template<class Type>
template<class... Args>
Type& Array<Type>::EmplaceBack(Args&&... args)
{
	if (m_size == m_capacity)
		Enlarge();

	return *(Type*)NEW_PLACEMENT(m_data + m_size++, Type)(Utils::Forward<Args>(args)...);
}


template<class Type>
Type Array<Type>::PopBack()
{
	ASSERT(m_size > 0);
	m_size--;
	Type result(m_data[m_size]);
	DELETE_PLACEMENT(m_data + m_size);
	return result;
}

template<class Type>
bool Array<Type>::Erase(const Type& value)
{
	for (size_t i = 0; i < m_size; ++i)
	{
		if (m_data[i] == value)
		{
			Erase(i);
			return true;
		}
	}
	return false;
}

template<class Type>
void Array<Type>::Erase(size_t index)
{
	ASSERT2(index < m_size, "Index out of bounds");
	DELETE_PLACEMENT(m_data + index);

	m_size--;

	NEW_PLACEMENT(m_data + index, Type)(Utils::Move(m_data[m_size]));
	//DELETE_PLACEMENT(m_data + m_size);
}


template<class Type>
void Array<Type>::EraseOrdered(size_t index)
{
	ASSERT2(index < m_size, "Index out of bounds");

	if (index < m_size - 1)
		memory::Move(m_data + index, m_data + index + 1, (m_size - index) * sizeof(Type));
	m_size--;
}


template<class Type>
bool Array<Type>::Find(const Type& value, size_t& index)
{
	for (size_t i = 0; i < m_size; ++i)
	{
		if (m_data[i] == value)
		{
			index = i;
			return true;
		}
	}
	return false;
}

template<class Type>
void Array<Type>::Swap(size_t index1, size_t index2)
{
	ASSERT2(index1 < m_size, "Index 1 out of bounds");
	ASSERT2(index2 < m_size, "Index 2 out of bounds");

	if (index1 == index2)
		return;

	Type tmp(Utils::Move(m_data[index1]));
	NEW_PLACEMENT(m_data + index1, Type)(Utils::Move(m_data[index2]));
	NEW_PLACEMENT(m_data + index2, Type)(Utils::Move(tmp));
}

template<class Type>
const Type& Array<Type>::operator[](size_t index) const
{
	ASSERT2(index < m_size, "Index out of bounds");
	return m_data[index];
}


template<class Type>
Type& Array<Type>::operator[](size_t index)
{
	ASSERT2(index < m_size, "Index out of bounds");
	return m_data[index];
}


template<class Type>
void Array<Type>::Reserve(size_t capacity)
{
	if (capacity <= m_capacity) return;

	m_capacity = capacity;
	Type* newData = static_cast<Type*>(m_allocator.Allocate(m_capacity * sizeof(Type), alignof(Type)));

	for (size_t i = 0; i < m_size; ++i)
	{
		NEW_PLACEMENT(newData + i, Type)(Utils::Move(m_data[i]));
	}

	if (m_data != nullptr)
		m_allocator.Deallocate(m_data);
	m_data = newData;
}


template<class Type>
void Array<Type>::Resize(size_t size)
{
	if (size == m_size && size == m_capacity) return;

	Type* newData = static_cast<Type*>(m_allocator.Allocate(size * sizeof(Type), alignof(Type)));

	size_t sizeMin = (size < m_size) ? size : m_size;
	for (size_t i = 0; i < sizeMin; ++i)
	{
		NEW_PLACEMENT(newData + i, Type)(Utils::Move(m_data[i]));
	}

	if (m_data != nullptr)
		m_allocator.Deallocate(m_data);
	m_data = newData;
	m_capacity = m_size = size;
}

template<class Type>
void Array<Type>::Compact()
{
	Resize(m_size);
}


template<class Type>
size_t Array<Type>::GetSize() const { return m_size; }

template<class Type>
size_t Array<Type>::GetCapacity() const { return m_capacity; }


template<class Type>
void Array<Type>::Enlarge()
{
	size_t newCapacity = (m_capacity == 0) ? INITIAL_SIZE : m_capacity * ENLARGE_MULTIPLIER;
	Reserve(newCapacity);
}


// hack to make interface clear


template<class Type>
inline Type* begin(Array<Type>& a)
{
	return a.Begin();
}


template<class Type>
inline Type* end(Array<Type>& a)
{
	return a.End();
}


template<class Type>
inline const Type* begin(const Array<Type>& a)
{
	return a.Begin();
}


template<class Type>
inline const Type* end(const Array<Type>& a)
{
	return a.End();
}


}