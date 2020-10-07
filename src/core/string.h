#pragma once

#include "core.h"
#include "allocator.h"


namespace Veng
{


namespace string
{


size_t Length(const char* str);

void Copy(char* destination, const char* source);
void Copy(char* destination, const char* source, size_t length);

int Compare(const char* str1, const char* str2);
int Compare(const char* str1, const char* str2, size_t num);

bool Equal(const char* str1, const char* str2);

char* FindChar(char* str, char character);
const char* FindChar(const char* str, char character);

char* FindCharR(char* str, char character);
const char* FindCharR(const char* str, char character);

char* FindStr(char* haystack, const char* needle);
const char* FindStr(const char* haystack, const char* needle);

char* FindStrR(char* haystack, const char* needle);
const char* FindStrR(const char* haystack, const char* needle);

}


class String
{
public:
	explicit String(Allocator& allocator);
	String(Allocator& allocator, const char* str);
	String(Allocator& allocator, const char* str, unsigned length);
	String(String&) = delete;
	String(String&& other);
	String& operator =(String&) = delete;
	String& operator =(String&& other);
	~String();

	char& operator[](unsigned index);
	const char& operator[](unsigned index) const;

	String& Cat(const char* str);
	String& Cat(const char* str, unsigned length);

	void Set(const char* str);
	void Set(const char* str, size_t length);

	const char* Cstr() const { return m_data; }
	size_t Length() const { return m_size; }

private:
	Allocator& m_allocator;
	char* m_data = nullptr;
	size_t m_size = 0;
};


template<size_t size>
class StaticInputBuffer
{
public:
	StaticInputBuffer() {}

	StaticInputBuffer(const char* str)
	{
		size_t len = string::Length(str);
		Add(str, len);
	}

	StaticInputBuffer(const StaticInputBuffer<size>& other)
		: StaticInputBuffer(other.m_data)
	{ }

	StaticInputBuffer(const StaticInputBuffer<size>& other, const char* str)
		: StaticInputBuffer(other.m_data)
	{
		size_t len = string::Length(str);
		Add(str, len)
	}

	~StaticInputBuffer() {}


	StaticInputBuffer& operator<<(const char* str)
	{
		while (str[0] != '\0')
		{
			if(m_ptr <= m_data + size)
			{
				m_ptr[0] = str[0];
				m_ptr++;
				str++;
			}
			else
			{
				ASSERT2(false, "Buffer overflow");
				break;
			}
		}
		return *this;
	}

	StaticInputBuffer& operator<<(const StaticInputBuffer& other)
	{
		return operator<<(other.m_data);
	}

	StaticInputBuffer operator+(const char* str)
	{
		return StaticInputBuffer<size>(this, str);
	}

	void Add(const char* str, size_t len)
	{
		if (len <= (size_t)(m_data + size - m_ptr))
		{
			string::Copy(m_ptr, str, len);
			m_ptr = m_ptr + len;
		}
		else
		{
			ASSERT2(false, "Buffer overflow");
		}
	}

	void Sub(size_t len)
	{
		if (m_ptr - len >= m_data)
		{
			m_ptr = m_ptr - len;
		}
		else
		{
			ASSERT2(false, "Buffer underflow");
		}
	}

	const char* Cstr() const { return m_data; }

	char* Data() { return &m_data[0]; }

private:
	char m_data[size + 1] = { '\0' };
	char* m_ptr = m_data;
};


}