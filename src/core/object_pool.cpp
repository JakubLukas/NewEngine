#include "object_pool.h"

namespace Veng
{


struct TestObjectPool
{
	TestObjectPool()
	{
		MainAllocator allocator;

		ObjectPool<int> objPool(allocator);

		int& object = objPool.GetObject();
		objPool.ReturnObject(object);
		ASSERT(objPool.GetSize() == 0);
		ASSERT(objPool.GetCapacity() == 64);
	}
};


static TestObjectPool s_test;


}