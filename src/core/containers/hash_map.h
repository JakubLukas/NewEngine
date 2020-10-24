#pragma once

#include "core/int.h"
#include "core/allocator.h"
#include "core/asserts.h"
#include "core/utility.h"


namespace Veng
{


template <class Key>
struct HashFunc
{
	static u32 get(const Key& key);
};

template <>
struct HashFunc<u32>
{
	static u32 get(const u32& key)
	{
		//TODO: just dummy value to test
		return key;
	}
};

template <>
struct HashFunc<char*>
{
	static u32 get(const char* key)
	{
		const char* k = key;
		//TODO: just dummy value to test
		u32 result = 1;
		while (*k != '\0')
		{
			result *= *k;
			k++;
		}
		return result;
	}
};

template <class T>
struct HashFunc<T*>
{
	static u32 get(const void* ptr)
	{
		uintptr p = (uintptr)ptr;
		return (u32)(p >> 32) * (u32)(p);
	}
};


template<class KeyType, class ValueType, class Hasher = HashFunc<KeyType>>
class HashMap final
{
public:
	using Map = HashMap<KeyType, ValueType>;
	using HashFunction = u32(*)(KeyType const&);

	struct HashNode
	{
		friend Map;

		HashNode(const KeyType& key, const ValueType& value);
		HashNode(const KeyType& key, const ValueType& value, int next);
		HashNode(const HashNode&) = delete;
		HashNode(HashNode&& other);
		HashNode& operator=(const HashNode&) = delete;
		HashNode& operator=(HashNode&& other);

		KeyType key;
		ValueType value;
	private:
		int next;
	};

public:
	explicit HashMap(Allocator& allocator);
	HashMap(HashMap&) = delete;
	HashMap(HashMap&& other);
	HashMap& operator =(HashMap&) = delete;
	HashMap& operator =(HashMap&& other);

	~HashMap();

	void Clear();

	HashNode* Begin();
	HashNode* End();
	const HashNode* Begin() const;
	const HashNode* End() const;

	bool Find(const KeyType& key, ValueType*& value) const;

	ValueType* Insert(const KeyType& key, const ValueType& value);

	bool Erase(const KeyType& key);

	void Rehash(unsigned bucketSize);

	size_t GetBucketsSize() const;

	size_t GetSize() const;


private:
	enum
	{
		MAX_FACTOR = 70,
		INITIAL_SIZE = 8,
		ENLARGE_MULTIPLIER = 2,
		INVALID_INDEX = -1,
	};

private:
	ValueType* Insert(unsigned bucketIdx, const KeyType& key, const ValueType& value);

	unsigned GetIndex(const KeyType& key) const;

	HashNode* GetNode(unsigned index);

private:
	Allocator& m_allocator;
	int* m_buckets = nullptr;
	unsigned m_bucketSize = 0;
	HashNode* m_table = nullptr;
	unsigned m_size = 0;
};


}


#include "internal/hash_map.inl"
