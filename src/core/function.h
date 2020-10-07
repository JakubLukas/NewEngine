#pragma once

#include "core/asserts.h"


namespace Veng
{

template <typename FunctionType>
class Function;


template<class ReturnType, class... Arguments>
class Function <ReturnType(Arguments...)>
{
public:
	using functionPtr = ReturnType(*)(Arguments...);

public:
	Function()
		: m_functionStub(nullptr)
		, m_instance(nullptr)
	{}

	Function(const Function<ReturnType(Arguments...)>&) = default;
	Function& operator=(const Function<ReturnType(Arguments...)>&) = default;

	Function(const Function<ReturnType(Arguments...)>&& other)
		: m_functionStub(other.m_functionStub)
		, m_instance(other.m_instance)
	{}
	Function& operator=(const Function<ReturnType(Arguments...)>&& other)
	{
		m_functionStub = other.m_functionStub;
		m_instance = other.m_instance;
	}


	template<ReturnType(*function)(Arguments...)>
	void Bind()
	{
		m_functionStub = &FunctionStub<function>;
	}


	template<class ObjectType, ReturnType(ObjectType::*method)(Arguments...)>
	void Bind(ObjectType* object)
	{
		m_functionStub = &ObjectStub<ObjectType, method>;
		m_instance = object;
	}


	void Clear()
	{
		m_functionStub = nullptr;
		m_instance = nullptr;
	}


	ReturnType operator()(Arguments... args)
	{
		ASSERT(m_functionStub != nullptr);
		return m_functionStub(m_instance, args...);
	}


private:
	using functionStubPtr = ReturnType(*)(void*, Arguments...);


	template<ReturnType(*function)(Arguments...)>
	static ReturnType FunctionStub(void*, Arguments... args)
	{
		return (function)(args...);
	}

	template<class ObjectType, ReturnType(ObjectType::*method)(Arguments...)>
	static ReturnType ObjectStub(void* instance, Arguments... args)
	{
		return (static_cast<ObjectType*>(instance)->*method)(args...);
	}


private:
	functionStubPtr m_functionStub;
	void* m_instance;
};



}