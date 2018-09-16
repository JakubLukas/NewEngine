#include "clob.h"

#include "core/memory.h"
#include "core/allocators.h"

#include <cstdlib>
#include <cerrno>
#include <cstdio>


namespace Veng
{


static bool IsWhiteSpace(const char& c)
{
	return (false
		|| c == ' '
		|| c == '\t'
		|| c == '\n'
		|| c == '\v'
		|| c == '\f'
		|| c == '\r'
		|| c == '\0'
		);
}


static bool IsEndOfLine(const char& c)
{
	return (c == '\r'
		|| c == '\n');
}



// INPUT CLOB

InputClob::InputClob(const char* data, size_t size)
	: m_data(data)
	, m_size(size)
	, m_position(0)
{

}


bool InputClob::Read(char* data, size_t size)
{
	SkipWhiteSpaces();
	if(m_position + size <= m_size)
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


bool InputClob::ReadString(char* data, size_t maxSize)
{
	SkipWhiteSpaces();
	size_t maxRead = (m_position + maxSize - 1 <= m_size) ? (m_position + maxSize - 1) : m_size;
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
			data[charsReaded] = '\0';
			return true;
		}
	}

	data[charsReaded] = '\0';
	return (charsReaded <= maxSize);
}


bool InputClob::ReadLine(char* data, size_t maxSize)
{
	SkipWhiteSpaces();
	size_t maxRead = (m_position + maxSize <= m_size) ? (m_position + maxSize) : m_size;
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


bool InputClob::Read(i32& value)
{
	SkipWhiteSpaces();
	char* endpoint = nullptr;
	const int BASE = 0;
	errno = 0;
	value = strtol(m_data + m_position, &endpoint, BASE);

	if (errno == 0)
	{
		m_position = endpoint - m_data;
		return true;
	}
	else
	{
		return false;
	}
}

bool InputClob::Read(u32& value)
{
	SkipWhiteSpaces();
	char* endpoint = nullptr;
	const int BASE = 0;
	errno = 0;
	value = strtoul(m_data + m_position, &endpoint, BASE);

	if (errno == 0)
	{
		m_position = endpoint - m_data;
		return true;
	}
	else
	{
		return false;
	}
}

bool InputClob::Read(float& value)
{
	SkipWhiteSpaces();
	char* endpoint = nullptr;
	errno = 0;
	value = strtof(m_data + m_position, &endpoint);

	if (errno == 0)
	{
		m_position = endpoint - m_data;
		return true;
	}
	else
	{
		return false;
	}
}

void InputClob::Skip(size_t size)
{
	ASSERT(m_position + size <= m_size);
	m_position += size;
}


void InputClob::SkipWhiteSpaces()
{
	for (; m_position < m_size; ++m_position)
	{
		if (!IsWhiteSpace(m_data[m_position]))
		{
			return;
		}
	}
}


size_t InputClob::GetSize() const
{
	return m_size;
}

void InputClob::SetPosition(size_t position)
{
	ASSERT(m_position + position <= m_size);
	m_position = position;
}

size_t InputClob::GetPosition() const
{
	return m_position;
}

const char* InputClob::GetData() const
{
	return m_data;
}



//


static void Reallocate(IAllocator& allocator, char*& ptr, size_t& size)
{
	if (size == 0)
	{
		size = 1024;
		ptr = (char*)allocator.Allocate(size, ALIGN_OF(char));
		return;
	}

	size_t newSize = size * 2;
	char* newPtr = (char*)allocator.Allocate(newSize, ALIGN_OF(char));
	memory::Move(newPtr, ptr, size);
	size = newSize;
	allocator.Deallocate(ptr);
	ptr = newPtr;
}


// OUTPUT CLOB

OutputClob::OutputClob(IAllocator& allocator)
	: m_allocator(allocator)
	, m_data(nullptr)
	, m_size(0)
	, m_position(0)
{

}

OutputClob::~OutputClob()
{
	m_allocator.Deallocate(m_data);
}


void OutputClob::Write(const char* data, size_t size)
{
	if (m_position + size > m_size)
		Reallocate(m_allocator, m_data, m_size);

	memory::Copy(m_data + m_position, data, size);
	m_position += size;
}


void OutputClob::WriteString(const char* data)
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


void OutputClob::WriteLine(const char* data)
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


void OutputClob::Write(i32 value)
{
	const size_t MAX_LENGTH = 20;
	if (m_position + MAX_LENGTH >= m_size)
		Reallocate(m_allocator, m_data, m_size);

	int writtenChars = snprintf(m_data + m_position, MAX_LENGTH, "%d", value);
	ASSERT(writtenChars > 0 && writtenChars < MAX_LENGTH);
	m_position += writtenChars;
}

void OutputClob::Write(u32 value)
{
	const size_t MAX_LENGTH = 20;
	if (m_position + MAX_LENGTH >= m_size)
		Reallocate(m_allocator, m_data, m_size);

	int writtenChars = snprintf(m_data + m_position, MAX_LENGTH, "%u", value);
	ASSERT(writtenChars > 0 && writtenChars < MAX_LENGTH);
	m_position += writtenChars;
}

void OutputClob::WriteHex(u32 value)
{
	const size_t MAX_LENGTH = 20;
	if (m_position + MAX_LENGTH >= m_size)
		Reallocate(m_allocator, m_data, m_size);

	int writtenChars = snprintf(m_data + m_position, MAX_LENGTH, "%#x", value);
	ASSERT(writtenChars > 0 && writtenChars < MAX_LENGTH);
	m_position += writtenChars;
}

void OutputClob::Write(float value)
{
	const size_t MAX_LENGTH = 20;
	if (m_position + MAX_LENGTH >= m_size)
		Reallocate(m_allocator, m_data, m_size);

	int writtenChars = snprintf(m_data + m_position, MAX_LENGTH, "%g", value);
	ASSERT(writtenChars > 0 && writtenChars < MAX_LENGTH);
	m_position += writtenChars;
}


size_t OutputClob::GetSize() const
{
	return m_position;
}

const char* OutputClob::GetData() const
{
	return m_data;
}


}