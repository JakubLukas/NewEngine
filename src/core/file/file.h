#pragma once

#include "core/file/path.h"
#include "core/file/file_system.h"
#include "core/function.h"


namespace Veng
{


typedef void* nativeAsyncHandle;
typedef void* nativeFileHandle;


namespace FS
{


struct Operation
{
	u64 internal;
	u64 internalHigh;
	u64 offset;
	void* hEvent;
};


bool CreateAsyncHandle(nativeAsyncHandle& asyncHandle);
bool DestroyAsyncHandle(nativeAsyncHandle asyncHandle);

bool OpenFile(nativeFileHandle& fileHandle, nativeAsyncHandle asyncHandle, const Path& path, const FileMode& mode);
bool CloseFile(nativeFileHandle fileHandle);
bool RemoveFile(const Path& path);

bool ReadFile(nativeFileHandle fileHandle, Operation* operation, size_t filePosition, void* buffer, size_t size);
bool WriteFile(nativeFileHandle fileHandle, Operation* operation, size_t filePosition, const void* data, size_t size);

size_t GetFileSize(nativeFileHandle fileHandle);

void QueryChanges(nativeAsyncHandle asyncHandle, Function<void(nativeFileHandle, size_t)> callback);

bool CreateDir(const Path& path);
bool RemoveDir(const Path& path);


//synchronous helper functions
bool OpenFileSync(nativeFileHandle& fileHandle, const Path& path, const FileMode& mode);
bool CloseFileSync(nativeFileHandle fileHandle);
bool ReadFileSync(nativeFileHandle fileHandle, size_t filePosition, void* buffer, size_t size, size_t& bytesRead);
bool WriteFileSync(nativeFileHandle fileHandle, size_t filePosition, const void* data, size_t size);

//---------- search ----------

using searchHandle = void*;
const searchHandle INVALID_SEARCH_HANDLE = nullptr;
struct SearchInfo
{
	enum class FileType : u8
	{
		Normal,
		Directory,
	};

	typedef u8 FileModifierFlags;
	enum FileModifierBits : FileModifierFlags
	{
		FileModifier_None = 0,
		FileModifier_Hidden = 1 << 0,
		FileModifier_ReadOnly = 1 << 1,
		FileModifier_SymLink = 1 << 2,
	};

	size_t fileSize;
	FileType type;
	FileModifierFlags modifiers;
	char fileName[Path::MAX_LENGTH] = { 0 };

};
searchHandle SearchFirstFile(const Path& path, SearchInfo& info);
bool SearchNextFile(searchHandle handle, SearchInfo& info);
void SearchClose(searchHandle handle);


}


}