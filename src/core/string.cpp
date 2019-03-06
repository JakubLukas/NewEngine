#include "string.h"

#include "asserts.h"
#include "memory.h"
#include <cstring>


namespace Veng
{


namespace string
{


size_t Length(const char* str)
{
	return strlen(str);
}


void Copy(char* destination, const char* source)
{
	while (*source != '\0')
	{
		*destination = *source;
		destination++;
		source++;
	}
	*destination = '\0';
}


void Copy(char* destination, const char* source, size_t length)
{
	memory::Copy(destination, source, length);
}


inline int Compare(const char* str1, const char* str2)
{
	return strcmp(str1, str2);
}

inline int Compare(const char* str1, const char* str2, size_t num)
{
	return strncmp(str1, str2, num);
}


bool Equal(const char* str1, const char* str2)
{
	return Compare(str1, str2) == 0;
}


char* FindChar(char* str, char character)
{
	return strchr(str, character);
}

const char* FindChar(const char* str, char character)
{
	return strchr(str, character);
}


char* FindCharR(char* str, char character)
{
	return strrchr(str, character);
}

const char* FindCharR(const char* str, char character)
{
	return strrchr(str, character);
}


char* FindStr(char* haystack, const char* needle)
{
	return strstr(haystack, needle);
}

const char* FindStr(const char* haystack, const char* needle)
{
	return strstr(haystack, needle);
}


char* FindStrR(char* haystack, const char* needle)
{
	size_t  haystackLen = Length(haystack);
	size_t  needleLen = Length(needle);
	char* s;

	if (needleLen > haystackLen)
		return nullptr;
	for (s = haystack + haystackLen - needleLen; s >= haystack; --s)
	{
		if (Compare(s, needle, needleLen) == 0)
			return s;
	}
	return nullptr;
}

const char* FindStrR(const char* haystack, const char* needle)
{
	return static_cast<const char*>(FindStrR(const_cast<char*>(haystack), needle));
}


}



String::String(IAllocator& allocator)
	: m_allocator(allocator)
{
}

String::String(IAllocator& allocator, const char* str)
	: m_allocator(allocator)
{
	m_size = string::Length(str);
	m_data = (char*)m_allocator.Allocate(m_size + 1, sizeof(char));
	string::Copy(m_data, str, m_size);
	m_data[m_size] = '\0';
}

String::String(IAllocator& allocator, const char* str, unsigned length)
	: m_allocator(allocator)
	, m_size(length)
{
	m_data = (char*)m_allocator.Allocate(m_size + 1, sizeof(char));
	string::Copy(m_data, str, m_size);
	m_data[m_size] = '\0';
}

String::String(String&& other)
	: m_allocator(other.m_allocator)
	, m_data(other.m_data)
	, m_size(other.m_size)
{
	other.m_data = nullptr;
	other.m_size = 0;
}

String& String::operator =(String&& other)
{
	char* data = m_data;
	size_t size = m_size;

	m_allocator = other.m_allocator;
	m_data = other.m_data;
	m_size = other.m_size;

	other.m_data = data;
	other.m_size = size;

	return *this;
}


String::~String()
{
	if(m_data != nullptr)
		m_allocator.Deallocate(m_data);
}


char& String::operator[](unsigned index)
{
	ASSERT(index >= 0 && index <= m_size);
	return m_data[index];
}


const char& String::operator[](unsigned index) const
{
	ASSERT(index >= 0 && index <= m_size);
	return m_data[index];
}


String& String::Cat(const char* str)
{
	if (str != nullptr && str[0] != '\0')
	{
		size_t strLen = string::Length(str);
		char* newData = (char*)m_allocator.Allocate(m_size + strLen + 1, sizeof(char));
		string::Copy(newData, m_data, m_size);
		string::Copy(newData + m_size, str, strLen);
		m_size += strLen;
		newData[m_size] = '\0';
		m_allocator.Deallocate(m_data);
		m_data = newData;
	}
	return *this;
}


String& String::Cat(const char* str, unsigned length)
{
	char* newData = (char*)m_allocator.Allocate(m_size + length + 1, sizeof(char));
	string::Copy(newData, m_data, m_size);
	string::Copy(newData + m_size, str, length);
	m_size += length;
	newData[m_size] = '\0';
	m_allocator.Deallocate(m_data);
	m_data = newData;

	return *this;
}


void String::Set(const char* str)
{
	size_t strLen = string::Length(str);

	if (m_size == strLen)
	{
		string::Copy(m_data, str, strLen);
	}
	else
	{
		m_size = strLen;
		if(m_data != nullptr)
			m_allocator.Deallocate(m_data);
		m_data = (char*)m_allocator.Allocate(m_size + 1, sizeof(char));
		string::Copy(m_data, str, m_size);
		m_data[m_size] = '\0';
	}
}


void String::Set(const char* str, unsigned length)
{
	if (m_size == length)
	{
		string::Copy(m_data, str, length);
	}
	else
	{
		m_size = length;
		if(m_data != nullptr)
			m_allocator.Deallocate(m_data);
		m_data = (char*)m_allocator.Allocate(m_size + 1, sizeof(char));
		string::Copy(m_data, str, m_size);
		m_data[m_size] = '\0';
	}
}


}