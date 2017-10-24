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


void MemCpy(void* destination, const void* source, size_t size);

void MemMove(void* destination, const void* source, size_t size);

void MemSet(void* destination, unsigned char value, size_t size);


}