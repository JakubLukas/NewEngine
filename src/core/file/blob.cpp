#include "blob.h"

#include "core/memory.h"
#include "core/allocator.h"
#include "core/asserts.h"


namespace Veng
{



// INPUT BLOB

InputBlob::InputBlob(const void* data, size_t size)
	: m_data(static_cast<const u8*>(data))
	, m_size(size)
	, m_position(0)
{

}


bool InputBlob::Read(void* data, size_t size)
{
	if(m_position + size <= m_size)
	{
		memory::Copy(data, m_data + m_position, size);
		m_position += size;
		return true;
	}
	else
	{
		return false;
	}
}

bool InputBlob::ReadString(char* buffer, size_t buffer_size)
{
	size_t strLen = 0;
	size_t pos = m_position;
	while (pos < m_size && m_data[pos++] != '\0') strLen++;

	if (strLen >= buffer_size || pos == m_size) return false;

	string::Copy(buffer, (char*)(m_data + m_position), strLen + 1);
	m_position += strLen + 1;

	return true;
}

bool InputBlob::ReadString(String& out_string)
{
	size_t strLen = 0;
	size_t pos = m_position;
	while (pos < m_size && m_data[pos++] != '\0') strLen++;

	if (pos == m_size) return false;

	out_string.Set((char*)(m_data + m_position), strLen);
	m_position += strLen + 1;

	return true;
}

void InputBlob::Skip(size_t size)
{
	ASSERT(m_position + size <= m_size);
	m_position += size;
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

const void* InputBlob::GetData() const
{
	return m_data;
}



//


static void Reallocate(Allocator& allocator, u8*& ptr, size_t& size)
{
	if (size == 0)
	{
		size = 1024;
		ptr = (u8*)allocator.Allocate(size, alignof(u8));
		return;
	}

	size_t newSize = size * 2;
	u8* newPtr = (u8*)allocator.Allocate(newSize, alignof(u8));
	memory::Move(newPtr, ptr, size);
	size = newSize;
	allocator.Deallocate(ptr);
	ptr = newPtr;
}


// OUTPUT BLOB

OutputBlob::OutputBlob(Allocator& allocator)
	: m_allocator(allocator)
	, m_data(nullptr)
	, m_size(0)
	, m_position(0)
{

}

OutputBlob::~OutputBlob()
{
	m_allocator.Deallocate(m_data);
}


void OutputBlob::Write(const void* data, size_t size)
{
	if (m_position + size > m_size)
		Reallocate(m_allocator, m_data, m_size);

	memory::Copy(m_data + m_position, data, size);
	m_position += size;
}

void OutputBlob::WriteAtPos(const void* data, size_t size, size_t pos)
{
	if (pos > m_position) {
		ASSERT2(false, "Position is outside of filled buffer");
		return;
	}

	if (pos + size < m_position) {
		memory::Copy(m_data + pos, data, size);
	}
	else {
		size_t overSize = m_position - pos;
		memory::Copy(m_data + pos, data, overSize);
		Write((u8*)data + overSize, size - overSize);
	}
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


size_t OutputBlob::GetSize() const
{
	return m_position;
}

size_t OutputBlob::GetPosition() const
{
	return m_position;
}

const void* OutputBlob::GetData() const
{
	return m_data;
}


}