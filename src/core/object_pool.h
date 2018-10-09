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
		while(m_batches != nullptr)
		{
			Batch* next = m_batches->next;
			DELETE_OBJECT(m_allocator, m_batches);
			m_batches = next;
		}
	}

	Type& GetObject()
	{
		Batch* batch = m_batches;
		while (batch != nullptr)
		{
			if(batch->size < BATCH_SIZE)
				break;
			else
				batch = batch->next;
		}

		if(batch == nullptr)
		{
			Enlarge();
			batch = m_batches;
			while(batch->next != nullptr)
			{
				batch = batch->next;
			}
		}

		Type& result = batch->data[batch->freeIdx].value;
		batch->freeIdx = batch->data[batch->freeIdx].next;
		batch->size++;
		m_size++;
		memory::Set(&result, 0, sizeof(Type));///////////////////////////
		return result;
	}

	void ReturnObject(Type& object)
	{
		Batch* batch = m_batches;
		while (batch != nullptr)
		{
			if (&batch->data[0].value <= &object && &object < &batch->data[0].value + BATCH_SIZE)
			{
				i32 idx = (i32)((&object - &batch->data[0].value) / sizeof(Type));
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

	void Compact()
	{
		Batch** prevNext = &m_batches;
		Batch* batch;
		while (batch != nullptr)
		{
			if (batch->size == 0)
			{
				prevNext = batch->next;
				DELETE_OBJECT(m_allocator, batch);
			}
			else
			{
				prevNext = &batch->next;
				batch = batch->next;
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
		if (m_batches == nullptr)
			m_batches = NEW_OBJECT(m_allocator, Batch)();

		Batch* batch = m_batches;
		while (batch->next != nullptr)
			batch = batch->next;

		batch->next = NEW_OBJECT(m_allocator, Batch)();
		m_batchCount++;

		batch = batch->next;
		for(int i = 0; i < BATCH_SIZE; ++i)
			batch->data[i].next = i + 1;
		batch->data[BATCH_SIZE - 1].next = -1;
	}

private:
	static const size_t BATCH_SIZE = 64;

	struct Batch
	{
		union
		{
			Type value;
			i32 next;
		} data[BATCH_SIZE];
		size_t size = 0;
		i32 freeIdx = 0;
		Batch* next = nullptr;
	};

private:
	IAllocator& m_allocator;
	Batch* m_batches;
	size_t m_size = 0;
	size_t m_batchCount = 0;
};


}