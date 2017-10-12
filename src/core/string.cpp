#include "string.h"

#include "asserts.h"
#include <cstring>


namespace Veng
{


int StrLength(const char* str)
{
	return (int)strlen(str);
}


void StrCopy(char* destination, const char* source, unsigned length)
{
	memcpy(destination, source, length);
}



String::String(IAllocator& allocator)
	: m_allocator(allocator)
{
}


String::String(const char* str, IAllocator& allocator)
	: m_allocator(allocator)
{
	m_size = StrLength(str);
	m_data = (char*)m_allocator.Allocate(m_size + 1);
	StrCopy(m_data, str, m_size);
	m_data[m_size] = '\0';
}


String::String(const char* str, unsigned length, IAllocator& allocator)
	: m_allocator(allocator)
	, m_size(length)
{
	m_data = (char*)m_allocator.Allocate(m_size + 1);
	StrCopy(m_data, str, m_size);
	m_data[m_size] = '\0';
}


String::~String()
{
	m_allocator.Deallocate(m_data);
}


char String::operator[](unsigned index)
{
	ASSERT(index >= 0 && index <= m_size);
	return m_data[index];
}


const char String::operator[](unsigned index) const
{
	ASSERT(index >= 0 && index <= m_size);
	return m_data[index];
}


String& String::Cat(const char* str)
{
	if (str != nullptr && str[0] != '\0')
	{
		unsigned strLen = StrLength(str);
		char* newData = (char*)m_allocator.Allocate(m_size + strLen + 1);
		StrCopy(newData, m_data, m_size);
		StrCopy(newData + m_size, str, strLen);
		m_size += strLen;
		newData[m_size] = '\0';
		m_allocator.Deallocate(m_data);
		m_data = newData;
	}
	return *this;
}


String& String::Cat(const char* str, unsigned length)
{
	char* newData = (char*)m_allocator.Allocate(m_size + length + 1);
	StrCopy(newData, m_data, m_size);
	StrCopy(newData + m_size, str, length);
	m_size += length;
	newData[m_size] = '\0';
	m_allocator.Deallocate(m_data);
	m_data = newData;

	return *this;
}


void String::Set(const char* str)
{
	unsigned strLen = StrLength(str);

	if (m_size == strLen)
	{
		StrCopy(m_data, str, strLen);
	}
	else
	{
		m_size = strLen;
		m_allocator.Deallocate(m_data);
		m_data = (char*)m_allocator.Allocate(m_size + 1);
		StrCopy(m_data, str, m_size);
		m_data[m_size] = '\0';
	}
}


void String::Set(const char* str, unsigned length)
{
	if (m_size == length)
	{
		StrCopy(m_data, str, length);
	}
	else
	{
		m_size = length;
		m_allocator.Deallocate(m_data);
		m_data = (char*)m_allocator.Allocate(m_size + 1);
		StrCopy(m_data, str, m_size);
		m_data[m_size] = '\0';
	}
}


}