#include "object_pool.h"


namespace Veng
{


struct Foo
{
	int i;
	Foo()
	{
		i = 5;
	}
};

struct TestObjectPool
{
	TestObjectPool()
	{
		MainAllocator allocator;

		ObjectPool<Foo> objPool(allocator);

		Foo* object = objPool.GetObject();
		objPool.ReturnObject(object);
		ASSERT(objPool.GetSize() == 0);
		ASSERT(objPool.GetCapacity() == 64);
	}
};


//TestObjectPool s_test;

void TestPool()
{
	TestObjectPool test;
}


}