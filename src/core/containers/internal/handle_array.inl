namespace Veng
{


template<class Type, class UnusedType = i32>
HandleArray::HandleArray(IAllocator& allocator)
{

}

template<class Type, class UnusedType = i32>
HandleArray::HandleArray(HandleArray&& other)
{

}

template<class Type, class UnusedType = i32>
HandleArray& HandleArray::operator =(const HandleArray&& other)
{

}

template<class Type, class UnusedType = i32>
HandleArray::~HandleArray()
{

}


template<class Type, class UnusedType = i32>
size_t HandleArray::Add(const Type& value)
{
	if(m_unusedCount > 0)
	{
		UnusedType idx = m_unused;
		m_unused = m_data[idx].next;
		NEW_PLACEMENT(m_data + idx, Type)(value);
#		if DEBUG_HANDLE_ARRAY
		m_data[idx].alive = true;
#		endif
		m_unusedCount--;
		return idx;
	}
	else
	{
		DataType& item = EmplaceBack(value);
#		if DEBUG_HANDLE_ARRAY
		item.alive = true;
#		endif
		return m_size - 1;
	}
}

template<class Type, class UnusedType = i32>
void HandleArray::Remove(size_t handle)
{
	size_t idx = handle;
	ASSERT2(idx < m_size, "Invalid handle");

#	if DEBUG_HANDLE_ARRAY
	ASSERT2(m_data[idx].alive, "Item has been already removed");
	m_data[idx].alive = false;
#	endif

	m_entities[idx].~Type();
	m_entities[idx].next = m_unused;
	m_unused = (UnusedType)idx;
	m_unusedCount++;
}


template<class Type, class UnusedType = i32>
template<class... Args>
Type& HandleArray::EmplaceBack(Args&&... args)
{
	if(m_size == m_capacity)
		Enlarge();

	NEW_PLACEMENT(m_data + m_size, Type)(Utils::Forward<Args>(args)...);

	return m_data[m_size++];
}


template<class Type, class UnusedType = i32>
void HandleArray::Enlarge()
{
	m_capacity = (m_capacity == 0) ? INITIAL_SIZE : m_capacity * ENLARGE_MULTIPLIER;
	
	Type* newData = static_cast<Type*>(m_allocator.Allocate(m_capacity * sizeof(Type), alignof(Type)));

	for(size_t i = 0; i < m_size; ++i)
	{
		NEW_PLACEMENT(newData + i, Type)(Utils::Move(m_data[i]));
		we cant move construct dead items
	}

	if(m_data != nullptr)
		m_allocator.Deallocate(m_data);
	m_data = newData;
}


}
