#pragma once

#include "core/int.h"
#include "core/iallocator.h"
#include "core/asserts.h"
#include "core/utility.h"


//#define DEBUG_HANDLE_ARRAY


namespace Veng
{


template<class Type, class UnusedType = i32>
class HandleArray final
{
public:
	explicit HandleArray(IAllocator& allocator);
	HandleArray(HandleArray&) = delete;
	HandleArray(HandleArray&& other);
	HandleArray& operator =(const HandleArray&) = delete;
	HandleArray& operator =(const HandleArray&& other);
	~HandleArray();

	size_t Add(const Type& value);
	size_t Add(const Type&& value);
	void Remove(size_t handle);

	Type& Get(size_t handle);
	const Type& Get(size_t handle) const;

	size_t GetSize() const;
	size_t GetCapacity() const;

private:
	static const size_t INITIAL_SIZE;
	static const float ENLARGE_MULTIPLIER;

	struct DataType
	{
		union Data {
			Type data;
			UnusedType next;
			Data() : next(-1) {}
		};
		Data data;
#		if defined(DEBUG_HANDLE_ARRAY)
		bool alive;
#		endif
	};

private:
	template<class... Args>
	DataType& EmplaceBack(Args&&... args);
	void Enlarge();

private:
	IAllocator& m_allocator;
	size_t m_capacity = 0;
	size_t m_size = 0;
	DataType* m_data = nullptr;
	UnusedType m_unused = 0;
	size_t m_unusedSize = 0;
};


}


#include "internal/handle_array.inl"
