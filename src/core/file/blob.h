#pragma once

#include "core/int.h"
#include "core/string.h"
#include "core/file/path.h"


namespace Veng
{

class Allocator;


class InputBlob
{
public:
	InputBlob(const void* data, size_t size);

	bool Read(void* data, size_t size);
	template<typename T> bool Read(T& value);
	template<> bool Read(String& value);
	template<> bool Read(Path& value);

	void Skip(size_t size);
	size_t GetSize() const;
	void SetPosition(size_t position);
	size_t GetPosition() const;
	const void* GetData() const;

private:
	bool ReadString(char* buffer, size_t buffer_size);
	bool ReadString(String& out_string);

private:
	const u8* m_data;
	size_t m_size;
	size_t m_position;
};

template<typename T> bool InputBlob::Read(T& value)
{
	return Read(&value, sizeof(T));
}

template<> bool InputBlob::Read(String& value)
{
	return ReadString(value);
}

template<> bool InputBlob::Read(Path& value)
{
	char buffer[Path::BUFFER_LENGTH];
	if (ReadString(buffer, Path::BUFFER_LENGTH)) {
		value.SetPath(buffer);
		return true;
	}
	return false;

}


//-----------------------------------------------------------------------------

class OutputBlob
{
public:
	OutputBlob(Allocator& allocator);
	~OutputBlob();

	void Write(const void* data, size_t size);
	void WriteAtPos(const void* data, size_t size, size_t pos);

	template<typename T> void Write(const T& value);
	template<> void Write(const char* const& value);
	template<> void Write(const String& value);
	template<> void Write(const Path& value);

	template<typename T> void WriteAtPos(const T& value, size_t pos);
	template<> void WriteAtPos(const String& value, size_t pos) { return; }

	size_t GetSize() const;
	size_t GetPosition() const;
	const void* GetData() const;

private:
	void WriteString(const char* data);

private:
	Allocator& m_allocator;
	u8* m_data;
	size_t m_size;
	size_t m_position;
};

template<typename T> void OutputBlob::Write(const T& value)
{
	Write(&value, sizeof(T));
}

template<> void OutputBlob::Write(const char* const& value)
{
	WriteString(value);
}

template<> void OutputBlob::Write(const String& value)
{
	WriteString(value.Cstr());
}

template<> void OutputBlob::Write(const Path& value)
{
	WriteString(value.GetPath());
}

template<typename T> void OutputBlob::WriteAtPos(const T& value, size_t pos)
{
	WriteAtPos(&value, sizeof(T), pos);
}

}