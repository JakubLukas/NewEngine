#pragma once


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
	{}


	void Bind(functionPtr function)
	{
		m_functionStub = &FunctionWrapStub;
		NEW_PLACEMENT(&m_wrap, FunctionWrap) { function };
	}


	template<class ObjectType>
	void Bind(ObjectType* object, ReturnType(ObjectType::*method)(Arguments...))
	{
		ASSERT(sizeof(ObjectWrap<ObjectType>) <= WRAP_SIZE);

		m_functionStub = &ObjWrapStub<ObjectType>;
		NEW_PLACEMENT(&m_wrap, ObjectWrap<ObjectType>) { object, method };
	}


	ReturnType operator()(Arguments... args)
	{
		ASSERT(m_wrap != nullptr);
		ASSERT(m_functionStub != nullptr);
		return m_functionStub(&m_wrap);
	}


private:
	using functionStubPtr = ReturnType(*)(void*, Arguments...);

	struct FunctionWrap
	{
		functionPtr function;
	};

	static ReturnType FunctionWrapStub(void* functionWrap, Arguments... args)
	{
		FunctionWrap* wrap = static_cast<FunctionWrap*>(functionWrap);
		return (wrap->function)(args...);
	}


	template<class T>
	struct ObjectWrap
	{
		using memberPtr = ReturnType(T::*)(Arguments...);

		T* instance;
		memberPtr method;
	};

	template<class ObjectType>
	static ReturnType ObjWrapStub(void* objWrap, Arguments... args)
	{
		ObjectWrap<ObjectType>* wrap = static_cast<ObjectWrap<ObjectType>*>(objWrap);
		return (wrap->instance->*(wrap->method))(args...);
	}


private:
	static const unsigned WRAP_SIZE = sizeof(uintptr) * 2;
	uintptr m_wrap[2] = {0};

	functionStubPtr m_functionStub;
};



}