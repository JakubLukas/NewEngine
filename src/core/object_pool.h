#pragma once

#include "allocators.h"


namespace Veng
{


template<class Type>
class ObjectPool
{
public:
	explicit ObjectPool(IAllocator& allocator)
		: m_allocator(allocator)
	{
	}

	~ObjectPool()
	{
		while(m_batch != nullptr)
		{
			Batch* next = m_batch->next;
			DELETE_OBJECT(m_batch);
			m_batch = next;
		}
	}

	Type& GetObject()
	{
		Batch* data = m_batch;
		while (data != nullptr)
		{
			if (data->size < BATCH_SIZE)
				return data->data[data->size++];
			else
				data = data->next;
		}

		Enlarge();

		Batch* data = m_batch;
		while (data->next != nullptr)
		{
			ASSERT(data->size == 0);
			return data->data[data->size++];
		}
	}

	void ReturnObject(Type& object)
	{
		Batch* data = m_batch;
		while (data != nullptr)
		{
			if (data->data <= &object && &object < data->data + BATCH_SIZE)
			{

			}
		}
	}

	void Compact()
	{
		Batch** prevNext = &m_batch;
		Batch* data;
		while (data != nullptr)
		{
			if (data->size == 0)
			{
				prevNext = data->next;
				DELETE_OBJECT(data);
			}
			else
			{
				prevNext = &data->next;
				data = data->next;
			}
		}
	}

	void Reserve(size_t capacity)
	{
		if (capacity < GetCapacity())
			return;

		size_t newBatchCount = (capacity + BATCH_SIZE - 1) / BATCH_SIZE;
		for (int i = 0, c = newBatchCount - m_batchCount; i < c; ++i)
			Enlarge();
	}

	size_t GetSize() const { return m_size; }

	size_t GetCapacity() const { return m_batchCount * BATCH_SIZE; }

private:
	void Enlarge()
	{
		if (m_batch == nullptr)
			m_batch = NEW_OBJECT(m_allocator, Batch)();

		Batch* batch = m_batch;
		while (batch->next != nullptr)
			batch = batch->next;

		batch->next = NEW_OBJECT(m_allocator, Batch)();
		m_batchCount++;

		batch = batch->next;
		for(int i = 0; i < BATCH_SIZE)
	}

private:
	static const size_t BATCH_SIZE = 64;

	struct Batch
	{
		Type data[BATCH_SIZE];
		size_t size = 0;
		size_t unused = (size_t)-1;
		Batch* next = nullptr;
	};

private:
	IAllocator& m_allocator;
	size_t m_batchCount = 0;
	Batch* m_batch;
};


}