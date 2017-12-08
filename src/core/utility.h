#pragma once


namespace Veng
{


namespace Utils
{


template<class Type> struct RemoveReference { typedef Type type; };
template<class Type> struct RemoveReference<Type&> { typedef Type type; };
template<class Type> struct RemoveReference<Type&&> { typedef Type type; };


template<class Type> struct IsLvalueReference { static constexpr bool value = false; };
template<class Type> struct IsLvalueReference<Type&> { static constexpr bool value = true; };


template<class Type> // forward an lvalue as either an lvalue or an rvalue
inline constexpr Type&& Forward(typename RemoveReference<Type>::type& arg)
{
	return static_cast<Type&&>(arg);
}

template<class Type> // forward an rvalue as an rvalue
inline constexpr Type&& Forward(typename RemoveReference<Type>::type&& arg)
{
	static_assert(!IsLvalueReference<Type>::value, "bad forward call");
	return static_cast<Type&&>(arg);
}


template<class Type>
inline constexpr typename RemoveReference<Type>::type&& Move(Type&& arg)
{
	return static_cast<typename RemoveReference<Type>::type&&>(arg);
}


}


}