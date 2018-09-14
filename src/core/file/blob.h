#pragma once

#include "core/int.h"


namespace Veng
{

class IAllocator;


class InputBlob
{
public:
	InputBlob(const void* data, size_t size);

	bool Read(void* data, size_t size);
	bool ReadString(char* data, size_t maxSize);
	bool Read(i32& value);
	bool Read(u32& value);
	bool Read(size_t& value);
	bool Read(float& value);
	void Skip(size_t size);
	size_t GetSize() const;
	void SetPosition(size_t position);
	size_t GetPosition() const;
	const void* GetData() const;

private:
	const unsigned char* m_data;
	size_t m_size;
	size_t m_position;
};


class OutputBlob
{
public:
	OutputBlob(IAllocator& allocator);

	void Write(const void* data, size_t size);
	void WriteString(const char* data);
	void Write(i32 value);
	void Write(u32 value);
	void Write(size_t value);
	void Write(float value);
	size_t GetSize() const;
	const void* GetData() const;

private:
	IAllocator& m_allocator;
	unsigned char* m_data;
	size_t m_size;
	size_t m_position;
};


}