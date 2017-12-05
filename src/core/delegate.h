#pragma once


namespace Veng
{


template <typename FunctionType>
class Function;


template<class ReturnType, class... Arguments>
class Function <ReturnType(Arguments...)>
{
private:
	typedef ReturnType(*functionStubPtr)(void*, Arguments...);

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



	template<class ObjectType>
	struct ObjectWrapper
	{
		typedef ReturnType(ObjectType::*methodPtr)(Arguments...);

		ObjectWrapper(ObjectType* instance, methodPtr method)
			: instance(instance)
			, method(method)
		{}


		ReturnType operator()(Arguments... args)
		{
			return (static_cast<ObjectType*>(instance)->*method)(args...);
		}


		ObjectType* instance;
		methodPtr method;
	};

	//template <class ObjectType>
	//using member_pair = ObjectWrapper<ObjectType>;




	template<class ObjectType, ReturnType(ObjectType::* function)(Arguments...) const>
	static ReturnType ObjectStubConst(void* instance, Arguments... args)
	{
		return (static_cast<ObjectType*>(instance)->*function)(args...);
	}

public:
	Function()
		: m_object(nullptr)
		, m_functionStub(nullptr)
	{}


	/*void Bind(ReturnType(*function)(Arguments...))
	{
		m_function = function;
		m_functionStub = &FunctionStub;
	}

	template<class ObjectType, ReturnType(ObjectType::*method)(Arguments...)>
	void Bind(ObjectType* object)
	{
		m_object = object;
		m_functionStub = &ObjectStub<ObjectType, method>;

	}

	template<class ObjectType, ReturnType(ObjectType::*method)(Arguments...) const>
	void Bind(ObjectType* object)
	{
		m_object = object;
		m_functionStub = &ObjectStubConst<ObjectType, method>;

	}*/

	template<class ObjectType>
	void Bind(ObjectType* object, ReturnType(ObjectType::*method)(Arguments...))
	{
		test = new ObjectWrapper<ObjectType>(object, method);
		//m_object = object;
		//m_functionStub = &ObjectStubConst<ObjectType, method>;

	}


	ReturnType operator()(Arguments... args)
	{
		//ASSERT(m_object != nullptr);
		//ASSERT(m_function != nullptr);
		//return (m_function)(m_object, m_function, args);
		return test();
	}

private:
	void* m_object;
	functionStubPtr m_functionStub;
	ObjectWrapper* test;
};



}