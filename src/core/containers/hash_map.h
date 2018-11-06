#pragma once

#include "core/int.h"
#include "core/allocators.h"
#include "core/asserts.h"
#include "core/utility.h"


namespace Veng
{


inline u32 HashU32(const u32& key)
{
	//TODO: just dummy value to test
	return key;
}

inline u32 HashChar(const char*& key)
{
	//TODO: just dummy value to test
	u32 result = 1;
	while (*key != '\0')
	{
		result *= *key;
		key++;
	}
	return result;
}


template<class KeyType, class ValueType>
class HashMap final
{
public:
	using Map = HashMap<KeyType, ValueType>;
	using HashFunction = u32(*)(const KeyType&);

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
	explicit HashMap(IAllocator& allocator, HashFunction hashFunc);
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

	unsigned GetBucketsSize() const;

	unsigned GetSize() const;


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
	IAllocator& m_allocator;
	HashFunction m_hashFunction;
	int* m_buckets = nullptr;
	unsigned m_bucketSize = 0;
	HashNode* m_table = nullptr;
	unsigned m_size = 0;
};


}


#include "hash_map.inl"
