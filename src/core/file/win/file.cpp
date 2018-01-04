#include "../file.h"

#include "../file_system.h"

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


/*FileSync::FileSync()
	: m_data(INVALID_HANDLE_VALUE)
{

}

FileSync::~FileSync()
{
	ASSERT2(m_data == INVALID_HANDLE_VALUE, "File wasn't closed\n");
}

bool FileSync::Open(const char* path, FileMode mode)
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

bool FileSync::Close()
{
	if (m_data != INVALID_HANDLE_VALUE)
	{
		bool result = (CloseHandle((HANDLE)m_data) != 0);
		m_data = INVALID_HANDLE_VALUE;
		return result;
	}
	return false;
}

bool FileSync::Flush()
{
	ASSERT(m_data != INVALID_HANDLE_VALUE);
	return (FlushFileBuffers((HANDLE)m_data) != 0);
}


bool FileSync::Read(void* buffer, size_t size)
{
	ASSERT(m_data != INVALID_HANDLE_VALUE);
	DWORD bytesReaded = 0;
	bool result = (::ReadFile(
		m_data,
		buffer,
		(DWORD)size,
		&bytesReaded,
		NULL
	) != 0);
	return result && size == bytesReaded;
}

bool FileSync::Write(void* data, size_t size)
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

bool FileSync::MovePosition(MoveMethod method, u64 position)
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

u64 FileSync::GetPosition() const
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

size_t FileSync::GetSize() const
{
	ASSERT(m_data != INVALID_HANDLE_VALUE);
	return GetFileSize(m_data, NULL);
}


bool FileSync::Exists(const char* path)
{
	DWORD attribs = GetFileAttributes(path);
	return (attribs != INVALID_FILE_ATTRIBUTES
		&& !(attribs & FILE_ATTRIBUTE_DIRECTORY));
}*/


}


namespace FS
{

static_assert(sizeof(OVERLAPPED)											== sizeof(Operation),				"Must be the same");
static_assert(sizeof(OVERLAPPED::Internal)									== sizeof(Operation::internal),		"Must be the same");
static_assert(sizeof(OVERLAPPED::InternalHigh)								== sizeof(Operation::internalHigh),	"Must be the same");
static_assert(sizeof(OVERLAPPED::Offset) + sizeof(OVERLAPPED::OffsetHigh)	== sizeof(Operation::offset),		"Must be the same");
static_assert(sizeof(OVERLAPPED::hEvent)									== sizeof(Operation::hEvent),		"Must be the same");


/*struct Operation : public OVERLAPPED
{
	static Operation* Create(IAllocator& allocator, size_t filePosition)
	{
		Operation foo;
		foo.hEvent = NULL;
		foo.Internal = 0;
		foo.InternalHigh = 0;
		foo.Offset = (DWORD)(filePosition & 0xFFFFffff);
		foo.OffsetHigh = (DWORD)(filePosition >> 32);
		foo.Pointer = NULL;
		return &foo;//bullshit, just saved code;
	}
};*/


bool CreateAsyncHandle(nativeAsyncHandle& asyncHandle)
{
	HANDLE hPort = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE, //create new port
		NULL, //create new port
		NULL, //ignored, so use NULL
		0 //as many concurrently running threads as there are processors in the system
	);

	if(hPort == NULL)
	{
		return false;
	}
	else
	{
		asyncHandle = hPort;
		return true;
	}
}


bool DestroyAsyncHandle(nativeAsyncHandle asyncHandle)
{
	ASSERT(asyncHandle != NULL);
	return (CloseHandle(asyncHandle) == TRUE);
}


bool OpenFile(nativeFileHandle& fileHandle, nativeAsyncHandle asyncHandle, const Path& path, const FileMode& mode)
{
	DWORD flags = FILE_ATTRIBUTE_NORMAL;
	flags |= FILE_FLAG_OVERLAPPED; //open for async use
	if(mode.flags & FileMode::FlagDeleteOnClose)
		flags |= FILE_FLAG_DELETE_ON_CLOSE;
	if(mode.flags & FileMode::FlagWriteThrough)
		flags |= FILE_FLAG_WRITE_THROUGH;

	HANDLE hFile = CreateFile(
		path.path,
		(mode.access == FileMode::Access::Read) ? GENERIC_READ : GENERIC_WRITE,
		shareModeTable[(unsigned)mode.shareMode],
		NULL,
		creationDispositionTable[(unsigned)mode.creationDisposition],
		flags,
		NULL
	);

	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	HANDLE hPort = CreateIoCompletionPort(
		hFile,
		asyncHandle,
		(ULONG_PTR)hFile, //user defined value
		0); //ignored

	if(hPort == NULL)
		return false;

	fileHandle = hFile;
	return true;
}


bool CloseFile(nativeFileHandle fileHandle)
{
	ASSERT(fileHandle != INVALID_HANDLE_VALUE);
	return (CloseHandle(fileHandle) == TRUE);
}


bool RemoveFile(const Path& path)
{
	return (DeleteFile(path.path) == TRUE);
}


bool ReadFile(nativeFileHandle fileHandle, Operation* operation, size_t filePosition, void* buffer, size_t size)
{
	ASSERT(fileHandle != INVALID_HANDLE_VALUE);

	OVERLAPPED* overlapped = reinterpret_cast<OVERLAPPED*>(operation);
	overlapped->Internal = 0;
	overlapped->InternalHigh = 0;
	overlapped->Offset = (DWORD)(filePosition & 0xFFFFffff);
	overlapped->OffsetHigh = (DWORD)(filePosition >> 32);
	overlapped->hEvent = NULL;

	BOOL result = ::ReadFile(
		fileHandle,
		buffer,
		(DWORD)size,
		NULL,
		overlapped
	);
	return (result == TRUE);
}


bool WriteFile(nativeFileHandle fileHandle, Operation* operation, size_t filePosition, void* data, size_t size)
{
	ASSERT(fileHandle != INVALID_HANDLE_VALUE);

	OVERLAPPED* overlapped = reinterpret_cast<OVERLAPPED*>(operation);
	overlapped->Internal = 0;
	overlapped->InternalHigh = 0;
	overlapped->Offset = (DWORD)(filePosition & 0xFFFFffff);
	overlapped->OffsetHigh = (DWORD)(filePosition >> 32);
	overlapped->hEvent = NULL;

	DWORD bytesWritten = 0;
	BOOL result = ::WriteFile(
		fileHandle,
		data,
		(DWORD)size,
		NULL,
		overlapped
	);
	return (result == TRUE);
}


size_t GetFileSize(nativeFileHandle fileHandle)
{
	LARGE_INTEGER size;
	ASSERT(::GetFileSizeEx(fileHandle, &size));
	return static_cast<size_t>(size.QuadPart);
}


void QueryChanges(nativeAsyncHandle asyncHandle, Function<void(nativeFileHandle, size_t)> callback)
{
	static const int OVERLAPPED_SIZE = 64;
	OVERLAPPED_ENTRY overlapped[OVERLAPPED_SIZE] = { 0 };
	ULONG count = 0;

	while(true)
	{
		BOOL result = ::GetQueuedCompletionStatusEx(
			(HANDLE)asyncHandle,
			overlapped,
			(ULONG)OVERLAPPED_SIZE,
			&count,
			0, //wait infinite time
			FALSE); //dunno

		if(result != FALSE)
		{
			for(ULONG i = 0; i < count; ++i)
			{
				nativeFileHandle hFile = reinterpret_cast<nativeFileHandle>(overlapped[i].lpCompletionKey);
				LPOVERLAPPED ol = overlapped[i].lpOverlapped;
				DWORD bytesTransferred = overlapped[i].dwNumberOfBytesTransferred;

				DWORD bytes_transfered = 0;
				result = GetOverlappedResult(
					hFile, //file
					ol, //overlapped
					&bytes_transfered,
					FALSE //nonblocking
				);

				if(result != 0)
				{
					callback(hFile, (size_t)bytesTransferred); // success
				}
				else
				{
					ASSERT(false);
				}
			}
		}
		else
		{
			DWORD err = GetLastError();

			switch(err)
			{
				case ERROR_ABANDONED_WAIT_0:
					ASSERT2(false, "CompletionPort handle was closed or is invalid");
					return;
				default:
					return;
			return; //no more finished file requests
		}
	}
}


}


}