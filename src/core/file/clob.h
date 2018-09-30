#pragma once

#include "core/int.h"


namespace Veng
{

class IAllocator;
class InputBlob;


class InputClob
{
public:
	InputClob(const char* data, size_t size);
	InputClob(InputBlob& blob);

	bool Read(char* data, size_t size);
	bool ReadString(char* data, size_t maxSize);
	bool ReadLine(char* data, size_t maxSize);
	bool Read(i32& value);
	bool Read(u32& value);
	bool Read(float& value);
	void Skip(size_t size);
	void SkipWhiteSpaces();
	size_t GetSize() const;
	void SetPosition(size_t position);
	size_t GetPosition() const;
	const char* GetData() const;

private:
	const char* m_data;
	size_t m_size;
	size_t m_position;
};


class OutputClob
{
public:
	OutputClob(IAllocator& allocator);
	~OutputClob();

	void Write(const char* data, size_t size);
	void WriteString(const char* data);
	void WriteLine(const char* data);
	void Write(i32 value);
	void Write(u32 value);
	void WriteHex(u32 value);
	void Write(float value);
	size_t GetSize() const;
	const char* GetData() const;

private:
	IAllocator& m_allocator;
	char* m_data;
	size_t m_size;
	size_t m_position;
};


}