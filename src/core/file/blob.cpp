#include "blob.h"

#include "core/memory.h"
#include "core/allocators.h"


namespace Veng
{


bool IsWhiteSpace(const char& c)
{
	return (c == '\t'
		|| c == '\r'
		|| c == '\n'
		|| c == '\0'
		|| c == ' ');
}


bool IsEndOfLine(const char& c)
{
	return (c == '\r'
		|| c == '\n');
}


size_t StrToInt(const char* str, int& num)
{
	ASSERT(str[0] == '+' || str[0] == '-' || (str[0] >= '0' && str[0] <= '9'));

	num = 0;
	size_t idx = 0;

	int sign = 1;
	if (str[idx] == '-')
	{
		sign = -1;
		idx++;
	}
	else if (str[idx] == '+')
	{
		idx++;
	}

	while (str[idx] >= '0' && str[idx] <= '9')
	{
		num = num * 10 + (str[idx] - '0');
		idx++;
	}
	num *= sign;

	return idx;
}


size_t StrToUIntHex(const char* str, u32& num)
{
	num = 0;
	size_t idx = 0;
	ASSERT(str[idx++] == '0');
	ASSERT(str[idx++] == 'x');

	while(true)
	{
		if(str[idx] >= '0' && str[idx] <= '9')
			num = num * 16 + (str[idx] - '0');
		else if(str[idx] >= 'A' && str[idx] <= 'F')
			num = num * 16 + (str[idx] - 'A' + 10);
		else if(str[idx] >= 'a' && str[idx] <= 'f')
			num = num * 16 + (str[idx] - 'a' + 10);
		else
			break;
		idx++;
	}

	return idx;
}


size_t StrToFloat(const char* str, float& num)
{
	ASSERT(str[0] == '+' || str[0] == '-' || (str[0] >= '0' && str[0] <= '9'));

	num = 0;
	size_t idx = 0;

	int sign = 1;
	if (str[idx] == '-')
	{
		sign = -1;
		idx++;
	}
	else if (str[idx] == '+')
	{
		idx++;
	}

	while (str[idx] >= '0' && str[idx] <= '9')
	{
		num = num * 10 + (str[idx] - '0');
		idx++;
	}

	if (str[idx] == '.')
	{
		idx++;
		float decPlace = 0.1f;
		while (str[idx] >= '0' && str[idx] <= '9')
		{
			num += (str[idx] - '0') * decPlace;
			decPlace *= 0.1f;
			idx++;
		}
	}
	num *= sign;

	return idx;
}



// INPUT BLOB

InputBlob::InputBlob(const void* data, size_t size)
	: m_data(static_cast<const char*>(data))
	, m_size(size)
	, m_position(0)
{

}


bool InputBlob::Read(void* data, size_t size)
{
	SkipWhiteSpaces();
	if(m_position + size < m_size)
	{
		memory::Copy(data, m_data + m_position, m_size);
		m_position += size;
		return true;
	}
	else
	{
		return false;
	}
}


bool InputBlob::ReadString(char* data, size_t maxSize)
{
	SkipWhiteSpaces();
	size_t maxRead = (m_position + maxSize < m_size) ? (m_position + maxSize) : m_size;
	size_t charsReaded = 0;
	for(size_t i = m_position; i < maxRead; ++i, ++charsReaded)
	{
		if (!IsWhiteSpace(m_data[m_position]))
		{
			data[charsReaded] = m_data[i];
			m_position++;
		}
		else
		{
			return true;
		}
	}

	return (charsReaded <= maxSize);
}


bool InputBlob::ReadLine(char* data, size_t maxSize)
{
	SkipWhiteSpaces();
	size_t maxRead = (m_position + maxSize < m_size) ? (m_position + maxSize) : m_size;
	size_t charsReaded = 0;
	for (size_t i = m_position; i < maxRead; ++i, ++charsReaded)
	{
		if (!IsEndOfLine(m_data[m_position]))
		{
			data[charsReaded] = m_data[i];
			m_position++;
		}
		else
		{
			return true;
		}
	}

	return (charsReaded <= maxSize);
}


bool InputBlob::Read(int& value)
{
	SkipWhiteSpaces();
	m_position += StrToInt(m_data + m_position, value);
	return true;
}

bool InputBlob::Read(float& value)
{
	SkipWhiteSpaces();
	m_position += StrToFloat(m_data + m_position, value);
	return true;
}

bool InputBlob::ReadHex(u32& value)
{
	SkipWhiteSpaces();
	m_position += StrToUIntHex(m_data + m_position, value);
	return true;
}


void InputBlob::Skip(size_t size)
{
	ASSERT(m_position + size < m_size);
	m_position += size;
}


void InputBlob::SkipWhiteSpaces()
{
	for (; m_position < m_size; ++m_position)
	{
		if (!IsWhiteSpace(m_data[m_position]))
		{
			return;
		}
	}
}


size_t InputBlob::GetSize() const
{
	return m_size;
}

void InputBlob::SetPosition(size_t position)
{
	ASSERT(m_position + position <= m_size);
	m_position = position;
}

size_t InputBlob::GetPosition() const
{
	return m_position;
}

const char* InputBlob::GetData() const
{
	return m_data;
}



//


static void Reallocate(IAllocator& allocator, void*& ptr, size_t& size)
{
	if (size == 0)
	{
		ptr = allocator.Allocate(1024, ALIGN_OF(char));
		return;
	}

	size_t newSize = size * 2;
	void* newPtr = allocator.Allocate(newSize, ALIGN_OF(char));
	memory::Move(newPtr, ptr, size);
	size = newSize;
	allocator.Deallocate(ptr);
	ptr = newPtr;
}


// OUTPUT BLOB

OutputBlob::OutputBlob(IAllocator& allocator)
	: m_allocator(allocator)
	, m_data(nullptr)
	, m_size(0)
	, m_position(0)
{

}

void OutputBlob::Write(const void* data, size_t size)
{
	if (m_position + size > m_size)
		Reallocate(m_allocator, m_data, m_size);

	memory::Copy(m_data + m_position, data, size);
	m_position += size;
}


void OutputBlob::WriteString(const char* data)
{
	while (data[0] != '\0')
	{
		if(m_position == m_size)
			Reallocate(m_allocator, m_data, m_size);
		m_data[m_position] = data[0];
		m_position++;
		data++;
	}
	if (m_position == m_size)
		Reallocate(m_allocator, m_data, m_size);
	m_data[m_position++] = '\0';
}


void OutputBlob::WriteLine(const char* data)
{
	while (data[0] != '\0')
	{
		if (m_position == m_size)
			Reallocate(m_allocator, m_data, m_size);
		m_data[m_position] = data[0];
		m_position++;
		data++;
	}
	if (m_position == m_size)
		Reallocate(m_allocator, m_data, m_size);
	m_data[m_position++] = '\n';
}


void OutputBlob::Write(int value)
{
	if(value < 0)
	{
		if(m_position == m_size)
			Reallocate(m_allocator, m_data, m_size);
		m_data[m_position++] = '-';
		value *= -1;
	}

	int digits = 0;
	while(value > 0)
	{
		if(m_position + digits == m_size)
			Reallocate(m_allocator, m_data, m_size);
		m_data[m_position + digits] = '0' + value % 10;
		digits++;
	}
	for(int i = 0; i < digits / 2; ++i)
	{
		m_data[m_position + i] = m_data[m_position + digits - i];
	}
	m_position += digits;
}

void OutputBlob::Write(float value)
{
	ASSERT2(false, "Not implemented yet");
}

void OutputBlob::WriteHex(u32 value)
{
	ASSERT2(false, "Not implemented yet");
}


size_t OutputBlob::GetSize() const
{
	return m_size;
}

const char* OutputBlob::GetData() const
{
	return m_data;
}


}