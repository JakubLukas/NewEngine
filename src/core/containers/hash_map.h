#pragma once

#include "core/int.h"
#include "core/allocators.h"
#include "core/asserts.h"


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

public:
	typedef HashMap<KeyType, ValueType, HashFunction> Map;


	template<class KeyType, class ValueType>
	struct HashNode
	{
		friend Map;

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

		HashNode(const HashNode<KeyType, ValueType>& other)
		{
			key = other.key;
			value = other.value;
			next = other.next;
		}

		HashNode<KeyType, ValueType>& operator=(const HashNode<KeyType, ValueType>& other)
		{
			key = other.key;
			value = other.value;
			next = other.next;
			return *this;
		}

		KeyType key;
		ValueType value;
	private:
		int next;
	};

	
	typedef HashNode<KeyType, ValueType> Node;

public:
	HashMap(IAllocator& allocator)
		: m_allocator(allocator)
	{}
	HashMap(HashMap&) = delete;
	HashMap& operator =(HashMap&) = delete;

	~HashMap()
	{
		for (unsigned i = 0; i < m_size; ++i)
		{
			DELETE_PLACEMENT(m_table + i);
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
			DELETE_PLACEMENT(m_table + i);
		}
		m_size = 0;
	}

	Node* Begin() { return m_table; }
	Node* End() { return m_table + m_size; }

	const Node* Begin() const { return m_table; }
	const Node* End() const { return m_table + m_size; }

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
		if ((m_size / (float)m_bucketSize) > MAX_FACTOR)
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
		while (node->next != INVALID_INDEX && node->key != key)
		{
			pointingIdx = &node->next;
			node = &m_table[node->next];
		}

		if (node->key != key)
			return false;

		int freeNodeIdx = *pointingIdx;
		*pointingIdx = node->next;

		DELETE_PLACEMENT(node);
		--m_size;

		if(m_size > 0)
		{
			//fill up hole in m_table
			Node& lastNode = m_table[m_size];
			unsigned lastNodeBucketIdx = GetIndex(lastNode.key);
			pointingIdx = &m_buckets[lastNodeBucketIdx];
			while(m_table[*pointingIdx].key != lastNode.key)
			{
				pointingIdx = &(m_table[*pointingIdx].next);
			}

			*pointingIdx = freeNodeIdx;

			NEW_PLACEMENT(node, Node)(lastNode);
			DELETE_PLACEMENT(&lastNode);
		}

		return true;
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
		void* data = m_allocator.Allocate(bucketSize * (sizeof(int) + sizeof(Node)) + ALIGN_OF(Node), ALIGN_OF(int));
		m_buckets = static_cast<int*>(data);
		m_table = static_cast<Node*>(AlignPointer(m_buckets + bucketSize, ALIGN_OF(Node)));

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

	unsigned BucketsSize() { return m_bucketSize; }

	unsigned GetSize() const { return m_size; }

private:
	ValueType* Insert(unsigned bucketIdx, const KeyType& key, const ValueType& value)
	{
		ASSERT(m_size < m_bucketSize);

		if (m_buckets[bucketIdx] == INVALID_INDEX)
		{
			Node* newNode = NEW_PLACEMENT(m_table + m_size, Node)(key, value);
			m_buckets[bucketIdx] = m_size++;
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
			Node* newNode = NEW_PLACEMENT(m_table + m_size, Node)(key, value);
			node.next = m_size++;
			return &newNode->value;
		}
	}

	unsigned GetIndex(const KeyType& key) const
	{
		auto hash = HashFunction::Get(key);
		return hash % m_bucketSize;
	}

	Node* GetNode(unsigned index)
	{
		ASSERT(index < m_bucketSize);
		return &m_table[index];
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


template<class KeyType, class ValueType, class HashFunction>
typename HashMap<KeyType, ValueType, HashFunction>::Node* begin(HashMap<KeyType, ValueType, HashFunction>& a)
{
	return a.Begin();
}


template<class KeyType, class ValueType, class HashFunction>
typename HashMap<KeyType, ValueType, HashFunction>::Node* end(HashMap<KeyType, ValueType, HashFunction>& a)
{
	return a.End();
}


template<class KeyType, class ValueType, class HashFunction>
typename const HashMap<KeyType, ValueType, HashFunction>::Node* begin(const HashMap<KeyType, ValueType, HashFunction>& a)
{
	return a.Begin();
}


template<class KeyType, class ValueType, class HashFunction>
typename const HashMap<KeyType, ValueType, HashFunction>::Node* end(const HashMap<KeyType, ValueType, HashFunction>& a)
{
	return a.End();
}


}