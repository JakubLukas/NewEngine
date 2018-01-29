#pragma once

#include "core/int.h"
#include "core/asserts.h"


namespace Veng
{


class InputBlob
{
public:
	InputBlob(const void* data, size_t size);

	bool Read(void* data, size_t size);
	bool ReadString(char* data, size_t maxSize);
	template<class Type> bool Read(Type& value);
	template<class Type> Type Read();
	void Skip(size_t size);
	void Trim();
	size_t GetSize() const;
	void SetPosition(size_t position);
	size_t GetPosition() const;
	const void* GetData() const;

private:
	const void* m_data;
	size_t m_size;
	size_t m_position;
};


template<class Type> bool InputBlob::Read(Type& value)
{
	return Read(&value, sizeof(Type));
}


template<class Type> Type InputBlob::Read()
{
	Type value;
	Read(value);
	return value;
}


}