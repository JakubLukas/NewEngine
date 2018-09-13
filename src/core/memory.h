#pragma once


namespace Veng
{


template <class Type> struct remove_reference
{
	typedef Type type;
};


template <class Type> struct remove_reference<Type&>
{
	typedef Type type;
};

template <class Type> struct remove_reference<Type&&>
{
	typedef Type type;
};

template <class Type>
inline Type&& forward(typename remove_reference<Type>::type& Arg)
{
	return (static_cast<Type&&>(Arg));
}


namespace memory
{


void Copy(void* destination, const void* source, size_t size);

void Move(void* destination, const void* source, size_t size);

void Set(void* destination, unsigned char value, size_t size);

void Swap(void* source1, void* source2, size_t size);


}


}