#pragma once

#include "core/iallocator.h"
#include "core/asserts.h"
#include "core/utility.h"
#include "core/memory.h"


namespace Veng
{


template<class Type>
class Array final
{
public:
	explicit Array(IAllocator& allocator);
	Array(Array&) = delete;
	Array(Array&& other);
	Array& operator =(Array&) = delete;
	Array& operator =(Array&& other);
	~Array();

	void Clear();

	Type* Begin();
	Type* End();
	const Type* Begin() const;
	const Type* End() const;


	Type& PushBack();
	Type& PushBack(const Type& value);

	Type& AddOrdered(const Type& value);

	template<class... Args>
	Type& EmplaceBack(Args&&... args);


	Type PopBack();

	bool Erase(const Type& value);
	void Erase(size_t idx);

	void EraseOrdered(size_t index);


	bool Find(const Type& value, size_t& idx);

	void Swap(size_t index1, size_t index2);

	const Type& operator[](size_t index) const;
	Type& operator[](size_t index);


	void Reserve(size_t capacity);
	void Resize(size_t size);
	void Compact();


	size_t GetSize() const;
	size_t GetCapacity() const;


private:
	enum
	{
		INITIAL_SIZE = 8,
		ENLARGE_MULTIPLIER = 2,
	};

private:
	void Enlarge();

private:
	IAllocator& m_allocator;
	size_t m_capacity = 0;
	size_t m_size = 0;
	Type* m_data = nullptr;
};


}


#include "internal/array.inl"
