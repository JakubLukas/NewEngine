#pragma once


namespace Veng
{


typedef void* nativeAsyncHandle;
typedef void* nativeFileHandle;


namespace FS
{


struct Operation;


bool CreateAsyncHandle(nativeAsyncHandle& asyncHandle);
bool DestroyAsyncHandle(nativeAsyncHandle asyncHandle);

bool OpenFile(nativeFileHandle& fileHandle, nativeAsyncHandle asyncHandle, const Path& path, const FileMode& mode);
bool CloseFile(nativeFileHandle fileHandle);
bool RemoveFile(const Path& path);

bool ReadFile(nativeFileHandle fileHandle, Operation* operation, size_t filePosition, void* buffer, size_t size);
bool WriteFile(nativeFileHandle fileHandle, Operation* operation, size_t filePosition, void* data, size_t size);


}


}