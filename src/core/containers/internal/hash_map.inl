
namespace Veng
{


template<class KeyType, class ValueType, class Hasher>
HashMap<KeyType, ValueType, Hasher>::HashNode::HashNode(const KeyType& key, const ValueType& value)
	: key(key)
	, value(value)
	, next(INVALID_INDEX)
{}

template<class KeyType, class ValueType, class Hasher>
HashMap<KeyType, ValueType, Hasher>::HashNode::HashNode(const KeyType& key, const ValueType& value, int next)
	: key(key)
	, value(value)
	, next(next)
{}

template<class KeyType, class ValueType, class Hasher>
HashMap<KeyType, ValueType, Hasher>::HashNode::HashNode(HashNode&& other)
	: key(other.key)
	, value(other.value)
	, next(other.next)
{
	other.next = INVALID_INDEX;
}

template<class KeyType, class ValueType, class Hasher>
typename HashMap<KeyType, ValueType, Hasher>::HashNode& HashMap<KeyType, ValueType, Hasher>::HashNode::operator=(typename HashMap<KeyType, ValueType, Hasher>::HashNode&& other)
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


template<class KeyType, class ValueType, class Hasher>
HashMap<KeyType, ValueType, Hasher>::HashMap(Allocator& allocator)
	: m_allocator(allocator)
{}

template<class KeyType, class ValueType, class Hasher>
HashMap<KeyType, ValueType, Hasher>::HashMap(HashMap<KeyType, ValueType, Hasher>&& other)
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

template<class KeyType, class ValueType, class Hasher>
HashMap<KeyType, ValueType, Hasher>& HashMap<KeyType, ValueType, Hasher>::operator=(HashMap<KeyType, ValueType, Hasher>&& other)
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

template<class KeyType, class ValueType, class Hasher>
HashMap<KeyType, ValueType, Hasher>::~HashMap()
{
	for (unsigned i = 0; i < m_size; ++i)
	{
		DELETE_PLACEMENT(m_table + i);
	}
	if (m_buckets != nullptr)
		m_allocator.Deallocate(m_buckets);
}


template<class KeyType, class ValueType, class Hasher>
void HashMap<KeyType, ValueType, Hasher>::Clear()
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


template<class KeyType, class ValueType, class Hasher>
typename HashMap<KeyType, ValueType, Hasher>::HashNode* HashMap<KeyType, ValueType, Hasher>::Begin() { return m_table; }

template<class KeyType, class ValueType, class Hasher>
typename HashMap<KeyType, ValueType, Hasher>::HashNode* HashMap<KeyType, ValueType, Hasher>::End() { return m_table + m_size; }

template<class KeyType, class ValueType, class Hasher>
typename const HashMap<KeyType, ValueType, Hasher>::HashNode* HashMap<KeyType, ValueType, Hasher>::Begin() const { return m_table; }

template<class KeyType, class ValueType, class Hasher>
typename const HashMap<KeyType, ValueType, Hasher>::HashNode* HashMap<KeyType, ValueType, Hasher>::End() const { return m_table + m_size; }


template<class KeyType, class ValueType, class Hasher>
bool HashMap<KeyType, ValueType, Hasher>::Find(const KeyType& key, ValueType*& value) const
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


template<class KeyType, class ValueType, class Hasher>
ValueType* HashMap<KeyType, ValueType, Hasher>::Insert(const KeyType& key, const ValueType& value)
{
	if (m_bucketSize == 0)
		Rehash(INITIAL_SIZE);
	if ((m_size / (float)m_bucketSize) * 100 > MAX_FACTOR)
		Rehash(m_bucketSize * ENLARGE_MULTIPLIER);

	unsigned bucketIdx = GetIndex(key);
	return Insert(bucketIdx, key, value);
}

template<class KeyType, class ValueType, class Hasher>
bool HashMap<KeyType, ValueType, Hasher>::Erase(const KeyType& key)
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


template<class KeyType, class ValueType, class Hasher>
void HashMap<KeyType, ValueType, Hasher>::Rehash(unsigned bucketSize)
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


template<class KeyType, class ValueType, class Hasher>
size_t HashMap<KeyType, ValueType, Hasher>::GetBucketsSize() const { return m_bucketSize; }

template<class KeyType, class ValueType, class Hasher>
size_t HashMap<KeyType, ValueType, Hasher>::GetSize() const { return m_size; }


template<class KeyType, class ValueType, class Hasher>
ValueType* HashMap<KeyType, ValueType, Hasher>::Insert(unsigned bucketIdx, const KeyType& key, const ValueType& value)
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


template<class KeyType, class ValueType, class Hasher>
unsigned HashMap<KeyType, ValueType, Hasher>::GetIndex(const KeyType& key) const
{
	auto hash = Hasher::get(key);
	return hash % m_bucketSize;
}

template<class KeyType, class ValueType, class Hasher>
typename HashMap<KeyType, ValueType, Hasher>::HashNode* HashMap<KeyType, ValueType, Hasher>::GetNode(unsigned index)
{
	ASSERT(index < m_bucketSize);
	return &m_table[index];
}


template<class KeyType, class ValueType, class Hasher>
inline typename HashMap<KeyType, ValueType, Hasher>::HashNode* begin(HashMap<KeyType, ValueType, Hasher>& a)
{
	return a.Begin();
}


template<class KeyType, class ValueType, class Hasher>
inline typename HashMap<KeyType, ValueType, Hasher>::HashNode* end(HashMap<KeyType, ValueType, Hasher>& a)
{
	return a.End();
}


template<class KeyType, class ValueType, class Hasher>
inline typename const HashMap<KeyType, ValueType, Hasher>::HashNode* begin(const HashMap<KeyType, ValueType, Hasher>& a)
{
	return a.Begin();
}


template<class KeyType, class ValueType, class Hasher>
inline typename const HashMap<KeyType, ValueType, Hasher>::HashNode* end(const HashMap<KeyType, ValueType, Hasher>& a)
{
	return a.End();
}


}