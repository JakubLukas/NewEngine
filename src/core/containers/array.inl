namespace Veng
{


template<class Type>
Array<Type>::Array(IAllocator& allocator)
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
	m_allocator = other.m_allocator;
	m_capacity = other.m_capacity;
	m_size = other.m_size;
	m_data = other.m_data;

	other.m_capacity = 0;
	other.m_size = 0;
	other.m_data = nullptr;

	return *this
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
Type& Array<Type>::PushBack(const Type& value)
{
	if (m_size == m_capacity)
		Enlarge();

	NEW_PLACEMENT(m_data + m_size, Type)(value);

	return m_data[m_size++];
}

template<class Type>
Type& Array<Type>::PushBack()
{
	if (m_size == m_capacity)
		Enlarge();

	NEW_PLACEMENT(m_data + m_size, Type)();

	return m_data[m_size++];
}

template<class Type>
template<class... Args>
Type& Array<Type>::EmplaceBack(Args&&... args)
{
	if (m_size == m_capacity)
		Enlarge();

	NEW_PLACEMENT(m_data + m_size, Type)(Utils::Forward<Args>(args)...);

	return m_data[m_size++];
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
void Array<Type>::Erase(size_t idx)
{
	ASSERT(idx < m_size);
	DELETE_PLACEMENT(m_data + idx);

	m_size--;

	NEW_PLACEMENT(m_data + idx, Type)(Utils::Move(m_data[m_size]));
	//DELETE_PLACEMENT(m_data + m_size);
}

template<class Type>
bool Array<Type>::Erase(Type& value)
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
const Type& Array<Type>::operator[](size_t index) const
{
	ASSERT(index < m_size);
	return m_data[index];
}


template<class Type>
Type& Array<Type>::operator[](size_t index)
{
	ASSERT(index < m_size);
	return m_data[index];
}


template<class Type>
void Array<Type>::Reserve(size_t capacity)
{
	if (capacity <= m_capacity) return;

	m_capacity = capacity;
	Type* newData = static_cast<Type*>(m_allocator.Allocate(m_capacity * sizeof(Type), ALIGN_OF(Type)));

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

	Type* newData = static_cast<Type*>(m_allocator.Allocate(size * sizeof(Type), ALIGN_OF(Type)));

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