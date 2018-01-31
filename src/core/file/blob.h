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
	bool ReadLine(char* data, size_t maxSize);
	bool Read(int& value);
	bool Read(float& value);
	bool ReadHex(u32& value);
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


}