#pragma once

#include "core/int.h"
#include "core/iallocator.h"
#include "path.h"
#include "core/function.h"


namespace Veng
{


struct FileMode
{
	enum class Access : u8
	{
		Read = 0,
		Write,
	};

	enum class ShareMode : u8
	{
		NoShare = 0,
		ShareRead,
		ShareWrite,
	};

	enum class CreationDisposition : u8
	{
		CreateNew = 0, // if file not exists create, else fail
		CreateAlways, // if file not exists create, else overwrite existing
		OpenExisting, // if file exists open, else fail
		OpenAlways, // if file exists open, else create and open
		TruncateExisting, // if file exists truncate, else fail
	};

	enum : u32
	{
		FlagNone = 0x00,
		FlagDeleteOnClose = 0x01, // delete file on close
		FlagWriteThrough = 0x02, // don't cache write to buffer
	};


	Access access;
	ShareMode shareMode;
	CreationDisposition creationDisposition;
	//1 byte padding
	u32 flags;
};


enum class MoveMethod
{
	Begin = 0,
	Current,
	End,
};


enum class fileHandle : u64 {};


class FileSystem
{
public:
	static FileSystem* Create(IAllocator& allocator);
	static void Destroy(FileSystem* system, IAllocator& allocator);

public:
	virtual ~FileSystem() {}

	virtual bool OpenFile(fileHandle& handle, const Path& path, FileMode mode) = 0;
	virtual void CloseFile(fileHandle handle) = 0;

	virtual bool Read(fileHandle handle, void* buffer, size_t size, Function<void(fileHandle)> callback) = 0;
	virtual bool Write(fileHandle handle, void* data, size_t size, Function<void(fileHandle)> callback) = 0;

	virtual void SetPosition(fileHandle handle, MoveMethod method, size_t position) = 0;
	virtual size_t GetPosition(fileHandle handle) const  = 0;

	virtual size_t GetSize(fileHandle handle) const = 0;

	virtual void Update(float deltaTime) = 0;

	virtual const Path& GetCurrentDir() const = 0;

	virtual bool CreateDirectory(const Path& path) const = 0;
	virtual bool RemoveDirectory(const Path& path) const = 0;

	/*
	remove file
	*/
};


}