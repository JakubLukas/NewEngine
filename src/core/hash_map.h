#pragma once

#include "int.h"
#include "allocators.h"


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


template<class KeyType, class ValueType, class HashFunction = HashCalculator<KeyType>>
class HashMap
{
public:
	HashMap(IAllocator& allocator)
		: m_allocator(allocator)
	{}
	~HashMap()
	{
	}

private:
	IAllocator& m_allocator;
};


}