#pragma once

#include "core/int.h"
#include "path.h"
#include "core/array.h"


namespace Veng
{


namespace FS
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
								  //Overlapped, //async
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


class File
{
public:
	File();
	~File();

	bool Open(const char* path, FileMode mode);
	bool Close();
	bool Flush();

	bool Read(void* buffer, size_t size);
	bool Write(void* data, size_t size);

	bool MovePosition(MoveMethod method, u64 position);
	u64 GetPosition() const;

	size_t GetSize() const;

	static bool Exists(const char* path);

private:
	void* m_data;
};


typedef u32 fileHandle;


class FileSystem
{
public:
	fileHandle OpenFile(const Path& path, FileMode mode);
	void CloseFile(fileHandle handle);

	bool Read(fileHandle handle, void* buffer, size_t size);
	bool Write(fileHandle handle, void* data, size_t size);
private:
	Array<void*> m_fileHandles;
};


}


}