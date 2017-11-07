#pragma once

#include "int.h"
#include "allocators.h"
#include "asserts.h"


namespace Veng
{

template<class KeyType>
struct HashCalculator
{
	static u32 Get(const KeyType& key);
};

template<>
struct HashCalculator<u32>
{
	static u32 Get(const u32& key)
	{
		//TODO: just dummy value to test
		return key;
	}
};

template<>
struct HashCalculator<const char*>
{
	static u32 Get(const char* key)
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
};


template<class KeyType, class ValueType, class HashFunction = HashCalculator<KeyType>>
class HashMap
{
private:
	static const float MAX_FACTOR;
	static const int INITIAL_SIZE = 8;
	static const int ENLARGE_MULTIPLIER = 2;
	static const int INVALID_INDEX = -1;

	template<class KeyType, class ValueType>
	struct HashNode
	{
		HashNode(const KeyType& key, const ValueType& value)
			: key(key)
			, value(value)
			, next(INVALID_INDEX)
		{}

		HashNode(const KeyType& key, const ValueType& value, int next)
			: key(key)
			, value(value)
			, next(next)
		{}

		HashNode& operator=(const HashNode& other)
		{
			key = other.key;
			value = other.value;
			next = other.next;
			return *this;
		}

		KeyType key;
		ValueType value;
		int next;
	};

	typedef HashNode<KeyType, ValueType> Node;

public:
	HashMap(IAllocator& allocator)
		: m_allocator(allocator)
	{}

	~HashMap()
	{
		for (unsigned i = 0; i < m_size; ++i)
		{
			m_table[i].~Node();
		}
		if (m_buckets != nullptr)
			m_allocator.Deallocate(m_buckets);
	}

	void Clear()
	{
		for (unsigned i = 0; i < m_bucketSize; ++i)
		{
			m_buckets[i] = INVALID_INDEX;
		}
		for (unsigned i = 0; i < m_size; ++i)
		{
			m_table[i]->~Node();
		}
		m_size = 0;
	}

	bool Find(const KeyType& key, ValueType*& value) const
	{
		if (m_size == 0)
			return false;

		unsigned bucketIdx = GetIndex(key);

		if (m_buckets[bucketIdx] == INVALID_INDEX)
			return false;

		Node& node = m_table[m_buckets[bucketIdx]];
		while (node.next != INVALID_INDEX && node.key != key)
		{
			node = m_table[node.next];
		}

		if (node.key == key)
		{
			value = &node.value;
			return true;
		}
		else
		{
			return false;
		}
	}

	ValueType* Insert(const KeyType& key, const ValueType& value)
	{
		if (m_bucketSize == 0)
			Rehash(INITIAL_SIZE);
		if ((m_size / m_bucketSize) > MAX_FACTOR)
			Rehash(m_bucketSize * ENLARGE_MULTIPLIER);

		unsigned bucketIdx = GetIndex(key);
		return Insert(bucketIdx, key, value);
	}

	bool Erase(const KeyType& key)
	{
		unsigned bucketIdx = GetIndex(key);

		if (m_buckets[bucketIdx] == INVALID_INDEX)
			return false;

		int* pointingIdx = &m_buckets[bucketIdx];
		Node* node = &m_table[*pointingIdx];
		while (node.next != INVALID_INDEX && node.key != key)
		{
			pointingIdx = &node->next;
			node = &m_table[node.next];
		}

		if (node.key != key)
			return false;

		*pointingIdx = node->next;

		node->~Node();

		Node& lastNode = m_table[m_size - 1];
		unsigned lastNodeIdx = GetIndex(lastNode.key);
		pointingIdx = &m_buckets[lastNodeIdx];
		node = &m_table[*pointingIdx];
		while (node.key != lastNode.key)
		{
			pointingIdx = &node->next;
			node = &m_table[node.next];
		}
		*pointingIdx = node->next;

		return true;
	}

	const ValueType& operator[](const KeyType& key) const
	{
		ValueType* value;
		if (Find(key, value))
		{
			return *value;
		}
		else
		{
			ASSERT2(false, "Key not found");
			return m_table[0];
		}
	}

	ValueType& operator[](const KeyType& key)
	{
		ValueType* value;
		if (Find(key, value))
		{
			return *value;
		}
		else
		{
			ASSERT2(false, "Key not found");
			return m_table[0];
		}
	}

	void Rehash(unsigned bucketSize)
	{
		ASSERT(bucketSize > m_bucketSize);

		int* oldBuckets = m_buckets;
		unsigned oldBucketSize = m_bucketSize;
		Node* oldTable = m_table;
		unsigned oldSize = m_size;

		m_bucketSize = bucketSize;
		m_size = 0;
		void* data = m_allocator.Allocate(bucketSize * (sizeof(int) + sizeof(Node)) + ALIGN_OF(Node), ALIGN_OF(Node));
		m_buckets = static_cast<int*>(data);
		m_table = static_cast<Node*>(AllignPointer(m_buckets + bucketSize, ALIGN_OF(Node)));

		for (unsigned i = 0; i < m_bucketSize; ++i)
		{
			m_buckets[i] = INVALID_INDEX;
		}
		for (unsigned i = 0; i < oldSize; ++i)
		{
			Node& node = oldTable[i];
			unsigned idx = GetIndex(node.key);
			Insert(idx, node.key, node.value);
		}

		if (oldBuckets != nullptr)
			m_allocator.Deallocate(oldBuckets);
	}

	unsigned BucketsSize() { return m_buckets; }

	unsigned GetSize() const { return m_size; }

private:
	ValueType* Insert(unsigned bucketIdx, const KeyType& key, const ValueType& value)
	{
		ASSERT(m_size < m_bucketSize);

		if (m_buckets[bucketIdx] == INVALID_INDEX)
		{
			Node* newNode = new (NewPlaceholder(), m_table + m_size) Node(key, value);
			m_buckets[bucketIdx] = m_size;
			++m_size;
			return &newNode->value;
		}

		Node& node = m_table[bucketIdx];
		while (node.key != key && node.next != INVALID_INDEX)
		{
			node = m_table[node.next];
		}

		if (node.key == key)
		{
			return nullptr;
		}
		else
		{
			Node* newNode = new (NewPlaceholder(), m_table + m_size) Node(key, value);
			node.next = m_size;
			++m_size;
			return &newNode->value;
		}
	}

	unsigned GetIndex(const KeyType& key) const
	{
		u32 hash = HashFunction::Get(key);
		return hash % m_bucketSize;
	}

private:
	IAllocator& m_allocator;
	int* m_buckets = nullptr;
	unsigned m_bucketSize = 0;
	Node* m_table = nullptr;
	unsigned m_size = 0;
};


template<class KeyType, class ValueType, class HashFunction>
const float HashMap<KeyType, ValueType, HashFunction>::MAX_FACTOR = 0.7f;


}