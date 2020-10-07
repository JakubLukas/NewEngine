#pragma once

#include "core/allocator.h"
#include "core/asserts.h"
#include "core/memory.h"


namespace Veng
{


template<class KeyType, class ValueType>
class AssociativeArray final
{
public:
	explicit AssociativeArray(Allocator& allocator);
	AssociativeArray(AssociativeArray&) = delete;
	AssociativeArray(AssociativeArray&& other);
	AssociativeArray& operator =(AssociativeArray&) = delete;
	AssociativeArray& operator =(AssociativeArray&& other);

	~AssociativeArray();

	void Clear();

	bool Find(const KeyType& key, ValueType*& value) const;

	ValueType* Begin();
	ValueType* End();
	const ValueType* Begin() const;
	const ValueType* End() const;

	ValueType* Insert(const KeyType& key, const ValueType& value);

	bool Erase(const KeyType& key);

	const ValueType& operator[](const KeyType& key) const;
	ValueType& operator[](const KeyType& key);

	void Reserve(size_t capacity);

	size_t GetSize() const;
	size_t GetCapacity() const;

	const KeyType* GetKeys() const;
	const ValueType* GetValues() const;

private:
	enum
	{
		INITIAL_SIZE = 4,
		ENLARGE_MULTIPLIER = 2,
	};

private:
	void Enlarge();
	size_t GetIndex(const KeyType& key) const;

private:
	Allocator& m_allocator;
	size_t m_capacity = 0;
	size_t m_size = 0;
	KeyType* m_keys = nullptr;
	ValueType* m_values = nullptr;
};


}


#include "internal/associative_array.inl"
