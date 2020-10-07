
namespace Veng
{


template<class KeyType, class ValueType>
HashMap<KeyType, ValueType>::HashNode::HashNode(const KeyType& key, const ValueType& value)
	: key(key)
	, value(value)
	, next(INVALID_INDEX)
{}

template<class KeyType, class ValueType>
HashMap<KeyType, ValueType>::HashNode::HashNode(const KeyType& key, const ValueType& value, int next)
	: key(key)
	, value(value)
	, next(next)
{}

template<class KeyType, class ValueType>
HashMap<KeyType, ValueType>::HashNode::HashNode(HashNode&& other)
	: key(other.key)
	, value(other.value)
	, next(other.next)
{
	other.next = INVALID_INDEX;
}

template<class KeyType, class ValueType>
typename HashMap<KeyType, ValueType>::HashNode& HashMap<KeyType, ValueType>::HashNode::operator=(typename HashMap<KeyType, ValueType>::HashNode&& other)
{
	KeyType k = key;
	ValueType v = value;

	key = other.key;
	value = other.value;
	next = other.next;

	other.key = k;
	other.value = v;
	other.next = INVALID_INDEX;

	return *this;
}


template<class KeyType, class ValueType>
HashMap<KeyType, ValueType>::HashMap(Allocator& allocator, HashFunction hashFunc)
	: m_allocator(allocator)
	, m_hashFunction(hashFunc)
{}

template<class KeyType, class ValueType>
HashMap<KeyType, ValueType>::HashMap(HashMap<KeyType, ValueType>&& other)
	: m_allocator(other.m_allocator)
	, m_buckets(other.m_buckets)
	, m_bucketSize(other.m_bucketSize)
	, m_table(other.m_table)
	, m_size(other.m_size)
{
	other.m_buckets = nullptr;
	other.m_bucketSize = 0;
	other.m_table = nullptr;
	other.m_size = 0;
}

template<class KeyType, class ValueType>
HashMap<KeyType, ValueType>& HashMap<KeyType, ValueType>::operator=(HashMap<KeyType, ValueType>&& other)
{
	int* buckets = m_buckets;
	unsigned bucketSize = m_bucketSize;
	HashNode* table = m_table;
	unsigned size = m_size;

	m_allocator = other.m_allocator;
	m_buckets = other.m_buckets;
	m_bucketSize = other.m_bucketSize;
	m_table = other.m_table;
	m_size = other.m_size;

	other.m_buckets = buckets;
	other.m_bucketSize = bucketSize;
	other.m_table = table;
	other.m_size = size;

	return *this;
}

template<class KeyType, class ValueType>
HashMap<KeyType, ValueType>::~HashMap()
{
	for (unsigned i = 0; i < m_size; ++i)
	{
		DELETE_PLACEMENT(m_table + i);
	}
	if (m_buckets != nullptr)
		m_allocator.Deallocate(m_buckets);
}


template<class KeyType, class ValueType>
void HashMap<KeyType, ValueType>::Clear()
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


template<class KeyType, class ValueType>
typename HashMap<KeyType, ValueType>::HashNode* HashMap<KeyType, ValueType>::Begin() { return m_table; }

template<class KeyType, class ValueType>
typename HashMap<KeyType, ValueType>::HashNode* HashMap<KeyType, ValueType>::End() { return m_table + m_size; }

template<class KeyType, class ValueType>
typename const HashMap<KeyType, ValueType>::HashNode* HashMap<KeyType, ValueType>::Begin() const { return m_table; }

template<class KeyType, class ValueType>
typename const HashMap<KeyType, ValueType>::HashNode* HashMap<KeyType, ValueType>::End() const { return m_table + m_size; }


template<class KeyType, class ValueType>
bool HashMap<KeyType, ValueType>::Find(const KeyType& key, ValueType*& value) const
{
	if (m_size == 0)
		return false;

	unsigned bucketIdx = GetIndex(key);

	if (m_buckets[bucketIdx] == INVALID_INDEX)
		return false;

	HashNode* node = &m_table[m_buckets[bucketIdx]];
	while (node->next != INVALID_INDEX && node->key != key)
	{
		node = &m_table[node->next];
	}

	if (node->key == key)
	{
		value = &node->value;
		return true;
	}
	else
	{
		return false;
	}
}


template<class KeyType, class ValueType>
ValueType* HashMap<KeyType, ValueType>::Insert(const KeyType& key, const ValueType& value)
{
	if (m_bucketSize == 0)
		Rehash(INITIAL_SIZE);
	if ((m_size / (float)m_bucketSize) * 100 > MAX_FACTOR)
		Rehash(m_bucketSize * ENLARGE_MULTIPLIER);

	unsigned bucketIdx = GetIndex(key);
	return Insert(bucketIdx, key, value);
}

template<class KeyType, class ValueType>
bool HashMap<KeyType, ValueType>::Erase(const KeyType& key)
{
	unsigned bucketIdx = GetIndex(key);

	if (m_buckets[bucketIdx] == INVALID_INDEX)
		return false;

	int* pointingIdx = &m_buckets[bucketIdx];
	HashNode* node = &m_table[*pointingIdx];
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
	m_size--;

	if(m_size > 0 && freeNodeIdx != m_size)
	{
		//fill up hole in m_table
		HashNode& lastNode = m_table[m_size];
		unsigned lastNodeBucketIdx = GetIndex(lastNode.key);
		pointingIdx = &m_buckets[lastNodeBucketIdx];
		while(m_table[*pointingIdx].key != lastNode.key)
		{
			pointingIdx = &(m_table[*pointingIdx].next);
		}

		*pointingIdx = freeNodeIdx;

		NEW_PLACEMENT(node, HashNode)(Utils::Move(lastNode));
		//DELETE_PLACEMENT(&lastNode);
	}

	return true;
}


template<class KeyType, class ValueType>
void HashMap<KeyType, ValueType>::Rehash(unsigned bucketSize)
{
	ASSERT(bucketSize > m_bucketSize);

	int* oldBuckets = m_buckets;
	unsigned oldBucketSize = m_bucketSize;
	HashNode* oldTable = m_table;
	unsigned oldSize = m_size;

	m_bucketSize = bucketSize;
	m_size = 0;
	void* data = m_allocator.Allocate(bucketSize * (sizeof(int) + sizeof(HashNode)) + alignof(HashNode), alignof(int));
	m_buckets = static_cast<int*>(data);
	m_table = static_cast<HashNode*>(AlignPointer(m_buckets + bucketSize, alignof(HashNode)));

	for (unsigned i = 0; i < m_bucketSize; ++i)
	{
		m_buckets[i] = INVALID_INDEX;
	}
	for (unsigned i = 0; i < oldSize; ++i)
	{
		HashNode& node = oldTable[i];
		unsigned idx = GetIndex(node.key);
		Insert(idx, node.key, node.value);
	}

	if (oldBuckets != nullptr)
		m_allocator.Deallocate(oldBuckets);
}


template<class KeyType, class ValueType>
size_t HashMap<KeyType, ValueType>::GetBucketsSize() const { return m_bucketSize; }

template<class KeyType, class ValueType>
size_t HashMap<KeyType, ValueType>::GetSize() const { return m_size; }


template<class KeyType, class ValueType>
ValueType* HashMap<KeyType, ValueType>::Insert(unsigned bucketIdx, const KeyType& key, const ValueType& value)
{
	ASSERT(m_size < m_bucketSize);

	if (m_buckets[bucketIdx] == INVALID_INDEX)
	{
		HashNode* newNode = NEW_PLACEMENT(m_table + m_size, HashNode)(key, value);
		m_buckets[bucketIdx] = m_size++;
		return &newNode->value;
	}

	HashNode* node = &m_table[m_buckets[bucketIdx]];
	while (node->key != key && node->next != INVALID_INDEX)
	{
		node = &m_table[node->next];
	}

	if (node->key == key)
	{
		return nullptr;
	}
	else
	{
		HashNode* newNode = NEW_PLACEMENT(m_table + m_size, HashNode)(key, value);
		node->next = m_size++;
		return &newNode->value;
	}
}


template<class KeyType, class ValueType>
unsigned HashMap<KeyType, ValueType>::GetIndex(const KeyType& key) const
{
	auto hash = m_hashFunction(key);
	return hash % m_bucketSize;
}

template<class KeyType, class ValueType>
typename HashMap<KeyType, ValueType>::HashNode* HashMap<KeyType, ValueType>::GetNode(unsigned index)
{
	ASSERT(index < m_bucketSize);
	return &m_table[index];
}


template<class KeyType, class ValueType>
inline typename HashMap<KeyType, ValueType>::HashNode* begin(HashMap<KeyType, ValueType>& a)
{
	return a.Begin();
}


template<class KeyType, class ValueType>
inline typename HashMap<KeyType, ValueType>::HashNode* end(HashMap<KeyType, ValueType>& a)
{
	return a.End();
}


template<class KeyType, class ValueType>
inline typename const HashMap<KeyType, ValueType>::HashNode* begin(const HashMap<KeyType, ValueType>& a)
{
	return a.Begin();
}


template<class KeyType, class ValueType>
inline typename const HashMap<KeyType, ValueType>::HashNode* end(const HashMap<KeyType, ValueType>& a)
{
	return a.End();
}


}