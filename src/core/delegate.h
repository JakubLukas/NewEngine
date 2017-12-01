#pragma once


namespace Veng
{


template <typename FunctionType>
class Function;

template<class ReturnType, class... Arguments>
class Function <ReturnType(Arguments...)>
{
public:
	typedef ReturnType(*functionPtr)(Arguments...);

public:
	Function()
		: m_functionPtr(nullptr)
	{}

	Function(functionPtr ptr)
		: m_functionPtr(ptr)
	{}

	Function(const Function& other)
		: m_functionPtr(other.m_functionPtr)
	{}

	Function(Function&& other)
		: m_functionPtr(other.m_functionPtr)
	{
		other.m_functionPtr = nullptr;
	}

	Function& operator=(const Function& other)
	{
		m_functionPtr = other.m_functionPtr;
	}

	Function& operator=(Function&& other)
	{
		m_functionPtr = other.m_functionPtr;
		other.m_functionPtr = nullptr;
	}


	ReturnType operator()(Arguments... args)
	{
		ASSERT(m_functionPtr != nullptr);
		return m_functionPtr(args);
	}

private:
	functionPtr m_functionPtr;
};


template<class ObjectType, class ReturnType, class... Arguments>
class Function <class ObjectType, ReturnType(Arguments...)>
{
public:
	typedef ReturnType(ObjectType::*functionPtr)(Arguments...);

public:
	Function()
		: m_objectPtr(nullptr)
		, m_functionPtr(nullptr)
	{}

	Function(ObjectType* object, functionPtr function)
		: m_objectPtr(object)
		, m_functionPtr(ptr)
	{}


	ReturnType operator()(Arguments... args)
	{
		ASSERT(m_objectPtr != nullptr);
		ASSERT(m_functionPtr != nullptr);
		return (m_objectPtr->*m_functionPtr)(args);
	}

private:
	ObjectType* m_objectPtr;
	functionPtr m_functionPtr;
};







struct Test
{
	int bar() { return 6; }
};

int foo() { return 6; }


template<int(*p)(void)>
int bind_foo(void* inst)
{
	return p();
}

template<class Type, int(Type::* p)(void)>
int bind_class(void* inst)
{
	return (((Type*)inst)->*p)();
}

template<class Type, class ret, class... args>
int bind_class2(Type* inst, ret(Type::* p)(args...))
{
	return (inst->*p)();
}

void mainaaa()
{
	void* inst;
	int(*pfnc)(void* inst);

	inst = nullptr;
	pfnc = &bind_class<Test, &Test::bar>;
	pfnc(inst);



	Test test;
	inst = &test;

	bind_class2(&test, &Test::bar);

}








}