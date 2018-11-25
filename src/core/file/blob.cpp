#include "blob.h"

#include "core/memory.h"
#include "core/iallocator.h"
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


bool InputBlob::ReadString(char* data, size_t maxSize)
{
	size_t maxRead = (m_position + maxSize - 1 <= m_size) ? (m_position + maxSize - 1) : m_size;
	size_t charsReaded = 0;
	while (charsReaded < maxRead && m_data[m_position] != '\0')
	{
		data[charsReaded++] = m_data[m_position++];
	}
	m_position++;//skip zero termination char

	data[charsReaded] = '\0';
	return (charsReaded <= maxSize);
}


bool InputBlob::Read(i32& value)
{
	if (m_position + sizeof(i32) <= m_size)
	{
		memory::Copy(&value, m_data + m_position, sizeof(i32));
		m_position += sizeof(i32);
		return true;
	}
	else
	{
		return false;
	}
}

bool InputBlob::Read(u32& value)
{
	if (m_position + sizeof(u32) <= m_size)
	{
		memory::Copy(&value, m_data + m_position, sizeof(u32));
		m_position += sizeof(u32);
		return true;
	}
	else
	{
		return false;
	}
}

bool InputBlob::Read(size_t& value)
{
	if (m_position + sizeof(size_t) <= m_size)
	{
		memory::Copy(&value, m_data + m_position, sizeof(size_t));
		m_position += sizeof(size_t);
		return true;
	}
	else
	{
		return false;
	}
}

bool InputBlob::Read(float& value)
{
	if (m_position + sizeof(float) <= m_size)
	{
		memory::Copy(&value, m_data + m_position, sizeof(float));
		m_position += sizeof(float);
		return true;
	}
	else
	{
		return false;
	}
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


static void Reallocate(IAllocator& allocator, u8*& ptr, size_t& size)
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

OutputBlob::OutputBlob(IAllocator& allocator)
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


void OutputBlob::Write(i32 value)
{
	if (m_position + sizeof(i32) >= m_size)
		Reallocate(m_allocator, m_data, m_size);

	memory::Copy(m_data + m_position, &value, sizeof(i32));
	m_position += sizeof(i32);
}

void OutputBlob::Write(u32 value)
{
	if (m_position + sizeof(u32) >= m_size)
		Reallocate(m_allocator, m_data, m_size);

	memory::Copy(m_data + m_position, &value, sizeof(u32));
	m_position += sizeof(u32);
}

void OutputBlob::Write(size_t value)
{
	if (m_position + sizeof(size_t) >= m_size)
		Reallocate(m_allocator, m_data, m_size);

	memory::Copy(m_data + m_position, &value, sizeof(size_t));
	m_position += sizeof(size_t);
}

void OutputBlob::Write(float value)
{
	if (m_position + sizeof(float) >= m_size)
		Reallocate(m_allocator, m_data, m_size);

	memory::Copy(m_data + m_position, &value, sizeof(float));
	m_position += sizeof(float);
}


size_t OutputBlob::GetSize() const
{
	return m_position;
}

const void* OutputBlob::GetData() const
{
	return m_data;
}


}