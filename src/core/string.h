#pragma once

#include "allocators.h"


namespace Veng
{


int StrLength(const char* str);

void StrCopy(char* destination, const char* source, unsigned length);

bool StrEqual(const char* str1, const char* str2);


template<int size>
class StaticString
{
public:

private:
	char m_data[size];
};


class String
{
public:
	explicit String(IAllocator& allocator);
	String(const char* str, IAllocator& allocator);
	String(const char* str, unsigned length, IAllocator& allocator);
	~String();

	char operator[](unsigned index);
	const char operator[](unsigned index) const;

	String& Cat(const char* str);
	String& Cat(const char* str, unsigned length);

	void Set(const char* str);
	void Set(const char* str, unsigned length);

	const char* Cstr() const { return m_data; }
	unsigned Length() const { return m_size; }

private:
	IAllocator& m_allocator;
	char* m_data = nullptr;
	unsigned m_size = 0;
};


}