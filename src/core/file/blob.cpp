#include "blob.h"

#include "core/memory.h"


namespace Veng
{


InputBlob::InputBlob(const void* data, size_t size)
	: m_data(data)
	, m_size(size)
	, m_position(0)
{

}


bool InputBlob::Read(void* data, size_t size)
{
	if(m_position + size <= m_size)
	{
		memory::Copy(data, static_cast<const char*>(m_data) + m_position, m_size);
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
	const char* src = static_cast<const char*>(m_data);
	size_t maxRead = (m_position + maxSize <= m_size) ? (m_position + maxSize) : m_size;
	for(size_t i = m_position, j = 0; i < maxRead; ++i, ++j)
	{
		if(src[i] != '\0' && src[i] != '\r' && src[i] != '\n')
		{
			data[j] = src[i];
			m_position++;
		}
		else
		{
			return true;
		}
	}

	return (maxRead <= maxSize);
}


void InputBlob::Skip(size_t size)
{
	ASSERT(m_position + size <= m_size);
	m_position += size;
}


void InputBlob::Trim()
{
	const char* src = static_cast<const char*>(m_data);
	for(size_t i = m_position; i < m_size; ++i)
	{
		if(src[i] == '\0' || src[i] == '\r' || src[i] == '\n')
			m_position++;
		else
			return;
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

const void* InputBlob::GetData() const
{
	return m_data;
}


}