namespace Veng
{


template<class Type>
ObjectPool<Type>::ObjectPool(Allocator& allocator)
	: m_allocator(allocator)
{
}

template<class Type>
ObjectPool<Type>::ObjectPool(ObjectPool&& other)
	: m_allocator(other.m_allocator)
	, m_batches(other.m_batches)
	, m_size(other.m_size)
	, m_batchCount(other.m_batchCount)
{
	other.m_batches = nullptr;
	other.m_size = 0;
	other.m_batchCount = 0;
}

template<class Type>
ObjectPool<Type>& ObjectPool<Type>::operator=(ObjectPool&& other)
{
	Batch* batches = m_batches;
	size_t size = m_size;
	size_t batchCount = m_batchCount;

	m_allocator = other.m_allocator;
	m_batches = other.m_batches;
	m_size = other.m_size;
	m_batchCount = other.m_batchCount;

	other.m_batches = batches;
	other.m_size = size;
	other.m_batchCount = batchCount;

	return *this;
}

template<class Type>
ObjectPool<Type>::~ObjectPool()
{
	while (m_batches != nullptr)
	{
		Batch* next = m_batches->next;
		DELETE_OBJECT(m_allocator, m_batches);
		m_batches = next;
	}
}


template<class Type>
Type* ObjectPool<Type>::GetObject()
{
	Batch* batch = m_batches;
	while (batch != nullptr)
	{
		if (batch->size < BATCH_SIZE)
			break;
		else
			batch = batch->next;
	}

	if (batch == nullptr)
	{
		Enlarge();
		batch = m_batches;
		while (batch->next != nullptr)
		{
			batch = batch->next;
		}
	}

	i32 nextIdx = batch->data[batch->freeIdx].next;
	Type* result = NEW_PLACEMENT(&batch->data[batch->freeIdx].value, Type)();
	batch->freeIdx = nextIdx;
	batch->size++;
	m_size++;
	return result;
}

template<class Type>
void ObjectPool<Type>::ReturnObject(Type* object)
{
	Batch* batch = m_batches;
	while (batch != nullptr)
	{
		if (&batch->data[0].value <= object && object < &batch->data[0].value + BATCH_SIZE)
		{
			i32 idx = (i32)(object - &batch->data[0].value);
			DELETE_PLACEMENT(object);
			batch->data[idx].next = batch->freeIdx;
			batch->freeIdx = idx;
			batch->size--;
			m_size--;
			return;
		}
		else
		{
			batch = batch->next;
		}
	}
	ASSERT2(false, "Object to return not found in internal batches");
}


template<class Type>
void ObjectPool<Type>::Compact()
{
	Batch** prevNext = &m_batches;
	Batch* batch;
	while (batch != nullptr)
	{
		if (batch->size == 0)
		{
			prevNext = batch->next;
			DELETE_OBJECT(m_allocator, batch);
			m_batchCount--;
		}
		else
		{
			prevNext = &batch->next;
			batch = batch->next;
		}
	}
}

template<class Type>
void ObjectPool<Type>::Reserve(size_t capacity)
{
	if (capacity < GetCapacity())
		return;

	size_t newBatchCount = (capacity + BATCH_SIZE - 1) / BATCH_SIZE;
	for (i32 i = 0, c = newBatchCount - m_batchCount; i < c; ++i)
		Enlarge();
}


template<class Type>
size_t ObjectPool<Type>::GetSize() const { return m_size; }

template<class Type>
size_t ObjectPool<Type>::GetCapacity() const { return m_batchCount * BATCH_SIZE; }


template<class Type>
void ObjectPool<Type>::Enlarge()
{
	Batch* batch;
	if (m_batches == nullptr)
	{
		m_batches = NEW_OBJECT(m_allocator, Batch)();
		batch = m_batches;
	}
	else
	{
		batch = m_batches;
		while (batch->next != nullptr)
			batch = batch->next;

		batch->next = NEW_OBJECT(m_allocator, Batch)();
		batch = batch->next;
	}

	m_batchCount++;

	for (i32 i = 0; i < BATCH_SIZE; ++i)
		batch->data[i].next = i + 1;
	batch->data[BATCH_SIZE - 1].next = -1;
}


}