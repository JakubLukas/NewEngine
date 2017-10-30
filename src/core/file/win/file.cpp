#include "core/file/file_system.h"

#define VC_EXTRALEAN
#include <windows.h>

#include "core/asserts.h"


namespace Veng
{


namespace FS
{


static_assert(sizeof(HANDLE) <= sizeof(void*), "Windows HANDLE must fit into File::m_data");


static DWORD shareModeTable[] =
{
	0,
	FILE_SHARE_READ,
	FILE_SHARE_WRITE,
};

static DWORD creationDispositionTable[] =
{
	CREATE_NEW,
	CREATE_ALWAYS,
	OPEN_EXISTING,
	OPEN_ALWAYS,
	TRUNCATE_EXISTING,
};

static DWORD moveMethodTable[] =
{
	FILE_BEGIN,
	FILE_CURRENT,
	FILE_END,
};


File::File()
	: m_data(INVALID_HANDLE_VALUE)
{

}

File::~File()
{
	ASSERT2(m_data == INVALID_HANDLE_VALUE, "File wasn't close");
}

bool File::Open(const char* path, FileMode mode)
{
	DWORD flags = FILE_ATTRIBUTE_NORMAL;
	if (mode.flags & FileMode::FlagDeleteOnClose)
		flags = flags | FILE_FLAG_DELETE_ON_CLOSE;
	if (mode.flags & FileMode::FlagWriteThrough)
		flags = flags | FILE_FLAG_WRITE_THROUGH;

	m_data = CreateFile(
		path,
		(mode.access == FileMode::Access::Read) ? GENERIC_READ : GENERIC_WRITE,
		shareModeTable[(unsigned)mode.shareMode],
		NULL,
		creationDispositionTable[(unsigned)mode.creationDisposition],
		flags,
		NULL
	);

	return (m_data != INVALID_HANDLE_VALUE);
}

bool File::Close()
{
	if (m_data != INVALID_HANDLE_VALUE)
	{
		bool result = (CloseHandle((HANDLE)m_data) != 0);
		m_data = INVALID_HANDLE_VALUE;
		return result;
	}
	return false;
}

bool File::Flush()
{
	ASSERT(m_data != INVALID_HANDLE_VALUE);
	return (FlushFileBuffers((HANDLE)m_data) != 0);
}


bool File::Read(void* buffer, size_t size)
{
	ASSERT(m_data != INVALID_HANDLE_VALUE);
	DWORD bytesReaded = 0;
	bool result = (ReadFile(
		m_data,
		buffer,
		(DWORD)size,
		&bytesReaded,
		NULL
	) != 0);
	return result && size == bytesReaded;
}

bool File::Write(void* data, size_t size)
{
	ASSERT(m_data != INVALID_HANDLE_VALUE);
	DWORD bytesWritten = 0;
	bool result = (WriteFile(
		m_data,
		data,
		(DWORD)size,
		&bytesWritten,
		NULL
	) != 0);
	return result && size == bytesWritten;
}

bool File::MovePosition(MoveMethod method, u64 position)
{
	ASSERT(m_data != INVALID_HANDLE_VALUE);
	LARGE_INTEGER pos;
	pos.QuadPart = (LONGLONG)position;
	return (SetFilePointerEx(
		m_data,
		pos,
		NULL,
		moveMethodTable[(unsigned)method]
	) != 0);
}

u64 File::GetPosition() const
{
	ASSERT(m_data != INVALID_HANDLE_VALUE);
	LARGE_INTEGER pos = {};
	LARGE_INTEGER filePointer;
	bool result = (SetFilePointerEx(
		m_data,
		pos,
		&filePointer,
		FILE_CURRENT
	) != 0);
	ASSERT(result);
	return filePointer.QuadPart;
}

size_t File::GetSize() const
{
	ASSERT(m_data != INVALID_HANDLE_VALUE);
	return GetFileSize(m_data, NULL);
}


bool File::Exists(const char* path)
{
	DWORD attribs = GetFileAttributes(path);
	return (attribs != INVALID_FILE_ATTRIBUTES
		&& !(attribs & FILE_ATTRIBUTE_DIRECTORY));
}


}


}