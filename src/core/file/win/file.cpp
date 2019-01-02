#include "../file.h"

#include <core/os/win/simple_windows.h>

#include "../file_system.h"
#include "core/asserts.h"
#include "core/memory.h"


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



static_assert(sizeof(OVERLAPPED) == sizeof(Operation), "Must be the same");
static_assert(sizeof(OVERLAPPED::Internal) == sizeof(Operation::internal), "Must be the same");
static_assert(sizeof(OVERLAPPED::InternalHigh) == sizeof(Operation::internalHigh), "Must be the same");
static_assert(sizeof(OVERLAPPED::Offset) + sizeof(OVERLAPPED::OffsetHigh) == sizeof(Operation::offset), "Must be the same");
static_assert(sizeof(OVERLAPPED::hEvent) == sizeof(Operation::hEvent), "Must be the same");



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
		path.GetPath(),
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
	return (DeleteFile(path.GetPath()) == TRUE);
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
	return (result == FALSE);//async ReadFile returns FALSE
	//if returns TRUE, operation isn't asynchronous ?
}


bool WriteFile(nativeFileHandle fileHandle, Operation* operation, size_t filePosition, const void* data, size_t size)
{
	ASSERT(fileHandle != INVALID_HANDLE_VALUE);

	OVERLAPPED* overlapped = reinterpret_cast<OVERLAPPED*>(operation);
	overlapped->Internal = 0;
	overlapped->InternalHigh = 0;
	overlapped->Offset = (DWORD)(filePosition & 0xFFFFffff);
	overlapped->OffsetHigh = (DWORD)(filePosition >> 32);
	overlapped->hEvent = NULL;

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

				if (result != FALSE)
				{
					callback(hFile, (size_t)bytesTransferred); // success
				}
				else
				{
					DWORD err = GetLastError();
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
					return; //no more finished file requests
			}
		}
	}
}


bool CreateDir(const Path& path)
{
	//SECURITY_ATTRIBUTES attr;
	BOOL result = CreateDirectory(
		path.GetPath(),
		NULL
	);

	if(result == FALSE)
	{
		DWORD err = GetLastError();

		switch(err)
		{
			case ERROR_PATH_NOT_FOUND:
				ASSERT2(false, "Invalid path");
				return false;
			case ERROR_ALREADY_EXISTS:
				return true;
			default:
				return true; //no more finished file requests
		}
	}
	return true;
}


bool RemoveDir(const Path& path)
{
	BOOL result = RemoveDirectory(
		path.GetPath()
	);

	if(result == FALSE)
	{
		DWORD err = GetLastError();
		ASSERT2(false, "Unexpected error");
		return false;
	}
	return true;
}



//synchronous helper functions
bool OpenFileSync(nativeFileHandle& fileHandle, const Path& path, const FileMode& mode)
{
	DWORD flags = FILE_ATTRIBUTE_NORMAL;
	if (mode.flags & FileMode::FlagDeleteOnClose)
		flags |= FILE_FLAG_DELETE_ON_CLOSE;
	if (mode.flags & FileMode::FlagWriteThrough)
		flags |= FILE_FLAG_WRITE_THROUGH;

	HANDLE hFile = CreateFile(
		path.GetPath(),
		(mode.access == FileMode::Access::Read) ? GENERIC_READ : GENERIC_WRITE,
		shareModeTable[(unsigned)mode.shareMode],
		NULL,
		creationDispositionTable[(unsigned)mode.creationDisposition],
		flags,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD err = GetLastError();
		return false;
	}

	fileHandle = hFile;
	return true;
}


bool CloseFileSync(nativeFileHandle fileHandle)
{
	ASSERT(fileHandle != INVALID_HANDLE_VALUE);
	return (CloseHandle(fileHandle) == TRUE);
}


bool ReadFileSync(nativeFileHandle fileHandle, size_t filePosition, void* buffer, size_t size, size_t& bytesRead)
{
	ASSERT(fileHandle != INVALID_HANDLE_VALUE);

	OVERLAPPED overlapped = { 0 };
	overlapped.Internal = 0;
	overlapped.InternalHigh = 0;
	overlapped.Offset = (DWORD)(filePosition & 0xFFFFffff);
	overlapped.OffsetHigh = (DWORD)(filePosition >> 32);
	overlapped.hEvent = NULL;

	DWORD sizeRead;
	BOOL result = ::ReadFile(
		fileHandle,
		buffer,
		(DWORD)size,
		&sizeRead,
		&overlapped
	);
	
	if(result != FALSE)
		bytesRead = sizeRead;

	return (result != FALSE);
}


bool WriteFileSync(nativeFileHandle fileHandle, size_t filePosition, const void* data, size_t size)
{
	ASSERT(fileHandle != INVALID_HANDLE_VALUE);

	OVERLAPPED overlapped = { 0 };
	overlapped.Internal = 0;
	overlapped.InternalHigh = 0;
	overlapped.Offset = (DWORD)(filePosition & 0xFFFFffff);
	overlapped.OffsetHigh = (DWORD)(filePosition >> 32);
	overlapped.hEvent = NULL;

	DWORD bytesWritten = 0;
	BOOL result = ::WriteFile(
		fileHandle,
		data,
		(DWORD)size,
		&bytesWritten,
		&overlapped
	);

	ASSERT(size == bytesWritten);

	return (result != FALSE);
}


searchHandle SearchFirstFile(const Path& path, SearchInfo& info)
{
	WIN32_FIND_DATA findData;
	HANDLE handle = ::FindFirstFile(path.GetPath(), &findData);
	if(handle == INVALID_HANDLE_VALUE)
	{
		ASSERT2(false, "FindFirstFile failed");
		DWORD err = GetLastError();
		return INVALID_SEARCH_HANDLE;
	}

	info.type = SearchInfo::FileType::Normal;
	info.modifiers = SearchInfo::FileModifiersBits::None;
	switch (findData.dwFileAttributes)
	{
	case FILE_ATTRIBUTE_DIRECTORY:
		info.type = SearchInfo::FileType::Directory;
		break;
	case FILE_ATTRIBUTE_REPARSE_POINT:
		if(findData.dwReserved0 == IO_REPARSE_TAG_SYMLINK)
			info.type = SearchInfo::FileType::Directory;
		break;
	case FILE_ATTRIBUTE_HIDDEN:
		info.modifiers |= SearchInfo::FileModifiersBits::Hidden;
		break;
	case FILE_ATTRIBUTE_READONLY:
		info.modifiers |= SearchInfo::FileModifiersBits::ReadOnly;
		break;
	}
	info.fileSize = ((size_t)findData.nFileSizeHigh << 32) + findData.nFileSizeLow;
	memory::Copy(info.fileName, findData.cFileName, Path::MAX_LENGTH);
	return handle;
}

bool SearchNextFile(searchHandle handle, SearchInfo& info)
{
	WIN32_FIND_DATA findData;
	if(::FindNextFile(handle, &findData) == FALSE)
	{
		DWORD err = GetLastError();
		if (err != ERROR_NO_MORE_FILES)
			ASSERT2(false, "FindNextFile failed");
		
		return false;
	}

	info.type = SearchInfo::FileType::Normal;
	info.modifiers = SearchInfo::FileModifiersBits::None;
	
	if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		info.type = SearchInfo::FileType::Directory;
	if (findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
	{
		if (findData.dwReserved0 == IO_REPARSE_TAG_SYMLINK)
			info.modifiers |= SearchInfo::FileModifiersBits::SymLink;
	}
	if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
		info.modifiers |= SearchInfo::FileModifiersBits::Hidden;
	if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		info.modifiers |= SearchInfo::FileModifiersBits::ReadOnly;

	info.fileSize = ((size_t)findData.nFileSizeHigh << 32) + findData.nFileSizeLow;
	memory::Copy(info.fileName, findData.cFileName, Path::MAX_LENGTH);

	return true;
}

void SearchClose(searchHandle handle)
{
	if(::FindClose(handle) == FALSE)
	{
		ASSERT2(false, "FindClose failed");
		DWORD err = GetLastError();
	}
}


}


}