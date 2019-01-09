namespace Veng
{

template<class Type, class UnusedType>
const size_t HandleArray<Type, UnusedType>::INITIAL_SIZE = 8;

template<class Type, class UnusedType>
const float HandleArray<Type, UnusedType>::ENLARGE_MULTIPLIER = 1.5f;



template<class Type, class UnusedType>
HandleArray<Type, UnusedType>::HandleArray(IAllocator& allocator)
	: m_allocator(allocator)
{
}

template<class Type, class UnusedType>
HandleArray<Type, UnusedType>::HandleArray(HandleArray&& other)
	: m_allocator(other.m_allocator)
	, m_capacity(other.m_capacity)
	, m_size(other.m_size)
	, m_data(other.m_data)
	, m_unused(other.m_unused)
	, m_unusedSize(other.m_unusedSize)
{
	other.m_capacity = 0;
	other.m_size = 0;
	other.m_data = nullptr;
	other.m_unused = 0;
	other.m_unusedSize = 0;
}

template<class Type, class UnusedType>
HandleArray<Type, UnusedType>& HandleArray<Type, UnusedType>::operator =(const HandleArray&& other)
{
	size_t capacity = m_capacity;
	size_t size = m_size;
	Type* data = m_data;
	UnusedType unused = m_unused;
	size_t unusedSize = m_unusedSize;

	m_allocator = other.m_allocator;

	m_capacity = other.m_capacity;
	m_size = other.m_size;
	m_data = other.m_data;
	m_unused = other.m_unused;
	m_unusedSize = other.m_unusedSize;

	other.m_capacity = capacity;
	other.m_size = size;
	other.m_data = data;
	other.m_unused = unused;
	other.m_unusedSize = unusedSize;

	return *this
}

template<class Type, class UnusedType>
HandleArray<Type, UnusedType>::~HandleArray()
{
	bool alive;
	UnusedType idx;
	for (size_t i = 0; i < m_size; ++i)
	{
		alive = true;
		idx = m_unused;
		for (size_t j = 0; j < m_unusedSize; ++j)
		{
			if ((size_t)idx == i)
			{
				alive = false;
				break;
			}
			idx = m_data[idx].next;
		}
		DELETE_PLACEMENT(m_data + i);
	}
	if(m_data != nullptr)
		m_allocator.Deallocate(m_data);
}


template<class Type, class UnusedType>
size_t HandleArray<Type, UnusedType>::Add(const Type& value)
{
	Add(Type(value));
}

template<class Type, class UnusedType>
size_t HandleArray<Type, UnusedType>::Add(const Type&& value)
{
	if(m_unusedSize > 0)
	{
		UnusedType idx = m_unused;
		m_unused = m_data[idx].next;
		NEW_PLACEMENT(m_data + idx, Type)(Utils::Move(value));
#		if defined(DEBUG_HANDLE_ARRAY)
		m_data[idx].alive = true;
#		endif
		m_unusedSize--;
		return idx;
	}
	else
	{
		DataType& item = EmplaceBack(value);
#		if defined(DEBUG_HANDLE_ARRAY)
		item.alive = true;
#		endif
		return m_size - 1;
	}
}

template<class Type, class UnusedType>
void HandleArray<Type, UnusedType>::Remove(size_t handle)
{
	size_t idx = handle;
	ASSERT2(idx < m_size, "Invalid handle");

#	if defined(DEBUG_HANDLE_ARRAY)
	ASSERT2(m_data[idx].alive, "Item has been already removed");
	m_data[idx].alive = false;
#	endif

	m_data[idx].data.~Type();
	m_data[idx].next = m_unused;
	m_unused = (UnusedType)idx;
	m_unusedSize++;
}

template<class Type, class UnusedType>
Type& HandleArray<Type, UnusedType>::Get(size_t handle)
{
	size_t idx = handle;
#	if defined(DEBUG_HANDLE_ARRAY)
	ASSERT2(m_data[idx].alive, "Item has been already removed");
#	endif
	return m_data[idx].data;
}

template<class Type, class UnusedType>
const Type& HandleArray<Type, UnusedType>::Get(size_t handle) const
{
	size_t idx = handle;
#	if defined(DEBUG_HANDLE_ARRAY)
	ASSERT2(m_data[idx].alive, "Item has been already removed");
#	endif
	return m_data[idx].data;
}


template<class Type, class UnusedType>
size_t HandleArray<Type, UnusedType>::GetSize() const
{
	return m_size - m_unusedSize;
}

template<class Type, class UnusedType>
size_t HandleArray<Type, UnusedType>::GetCapacity() const
{
	return m_capacity;
}


template<class Type, class UnusedType>
template<class... Args>
typename HandleArray<Type, UnusedType>::DataType& HandleArray<Type, UnusedType>::EmplaceBack(Args&&... args)
{
	if(m_size == m_capacity)
		Enlarge();

	m_size++;
	return *(DataType*)NEW_PLACEMENT(m_data + m_size, Type)(Utils::Forward<Args>(args)...);
}


template<class Type, class UnusedType>
void HandleArray<Type, UnusedType>::Enlarge()
{
	m_capacity = (m_capacity == 0) ? INITIAL_SIZE : (size_t)(m_capacity * ENLARGE_MULTIPLIER);
	
	DataType* newData = static_cast<DataType*>(m_allocator.Allocate(m_capacity * sizeof(Type), alignof(Type)));

	for(size_t i = 0; i < m_size; ++i)
	{
		DataType& item = *(DataType*)NEW_PLACEMENT(newData + i, Type)(Utils::Move(m_data[i].data));
#		if defined(DEBUG_HANDLE_ARRAY)
		item.alive = m_data[i].alive;
#		endif
	}

	if(m_data != nullptr)
		m_allocator.Deallocate(m_data);
	m_data = newData;
}


}
