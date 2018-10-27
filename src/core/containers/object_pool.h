#pragma once

#include "core/allocators.h"


namespace Veng
{


template<class Type>
class ObjectPool final
{
public:
	explicit ObjectPool(IAllocator& allocator);
	ObjectPool(ObjectPool&) = delete;
	ObjectPool(ObjectPool&& other);
	ObjectPool& operator =(ObjectPool&) = delete;
	ObjectPool& operator =(ObjectPool&& other);

	~ObjectPool();

	Type* GetObject();
	void ReturnObject(Type* object);

	void Compact();
	void Reserve(size_t capacity);

	size_t GetSize() const;
	size_t GetCapacity() const;


private:
	enum
	{
		BATCH_SIZE = 64,
	};

	struct Batch
	{
		union Data
		{
			i32 next;
			Type value;
			Data() : next(-1) {}
		};
		Data data[BATCH_SIZE];

		size_t size = 0;
		i32 freeIdx = 0;
		Batch* next = nullptr;
	};

private:
	void Enlarge();

private:
	IAllocator& m_allocator;
	Batch* m_batches = nullptr;
	size_t m_size = 0;
	size_t m_batchCount = 0;
};


}


#include "object_pool.inl"