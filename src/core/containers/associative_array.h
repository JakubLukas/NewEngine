#pragma once

#include "core/allocators.h"
#include "core/asserts.h"


namespace Veng
{


template<class KeyType, class ValueType>
class AssociativeArray
{
public:
	explicit AssociativeArray(IAllocator& allocator);
	AssociativeArray(AssociativeArray&) = delete;
	AssociativeArray(AssociativeArray&& other);
	AssociativeArray& operator =(AssociativeArray&) = delete;
	AssociativeArray& operator =(AssociativeArray&& other);

	~AssociativeArray();

	void Clear();

	bool Find(const KeyType& key, ValueType*& value) const;

	ValueType* begin();
	ValueType* end();
	const ValueType* begin() const;
	const ValueType* end() const;

	ValueType* Insert(const KeyType& key, const ValueType& value);

	bool Erase(const KeyType& key);


	const ValueType& operator[](const KeyType& key) const;
	ValueType& operator[](const KeyType& key);

	void Reserve(size_t capacity);

	size_t GetSize() const;
	size_t GetCapacity() const;

	KeyType* GetKeys() const;
	ValueType* GetValues() const;

private:
	void Enlarge();

	size_t GetIndex(const KeyType& key) const;

private:
	IAllocator& m_allocator;
	size_t m_capacity = 0;
	size_t m_size = 0;
	KeyType* m_keys = nullptr;
	ValueType* m_values = nullptr;
};


}


#include "associative_array.inl"