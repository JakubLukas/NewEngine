#pragma once

#include "allocators.h"


namespace Veng
{


namespace string
{


size_t Length(const char* str);

void Copy(char* destination, const char* source, size_t length);

bool Equal(const char* str1, const char* str2);


}


class String
{
public:
	explicit String(IAllocator& allocator);
	String(IAllocator& allocator, const char* str);
	String(IAllocator& allocator, const char* str, unsigned length);
	~String();

	char operator[](unsigned index);
	const char operator[](unsigned index) const;

	String& Cat(const char* str);
	String& Cat(const char* str, unsigned length);

	void Set(const char* str);
	void Set(const char* str, unsigned length);

	const char* Cstr() const { return m_data; }
	size_t Length() const { return m_size; }

private:
	IAllocator& m_allocator;
	char* m_data = nullptr;
	size_t m_size = 0;
};


}