#include "file_system.h"

#include "file.h"
#include "core/array.h"


namespace Veng
{





struct File
{
	nativeFileHandle handle;
	nativeAsyncHandle asyncHandle;
	FS::Operation* operation;
	Function<void()> callback;
	size_t position;
	i32 refCount;
};


class FileSystemImpl : public FileSystem
{
public:
	FileSystemImpl(IAllocator& allocator)
		: m_allocator(allocator)
		, m_files(m_allocator)
		, m_freeIndex(-1)
	{}


	~FileSystemImpl() override
	{}


	bool OpenFile(fileHandle& handle, const Path& path, FileMode mode) override
	{
		size_t idx;
		File* file;
		if (m_freeIndex != -1)
		{
			idx = m_freeIndex;
			m_freeIndex = (i64)m_files[idx].handle;
			file = &(m_files[idx]);
		}
		else
		{
			idx = m_files.GetSize();
			m_files.Push();
			file = &(m_files[idx]);
			if (!FS::CreateAsyncHandle(file->asyncHandle))
			{
				m_files.Pop();
				return false;
			}
		}

		if (FS::OpenFile(file->handle, file->asyncHandle, path, mode))
		{
			file->refCount++;
			handle = (fileHandle)idx;
			return true;
		}
		else
		{
			return false;
		}
	}


	void CloseFile(fileHandle handle) override
	{
		File& file = m_files[(size_t)handle];

		ASSERT(FS::CloseFile(file.handle));

		file.callback.Clear();
		file.position = 0;
		file.refCount--;
		file.handle = (nativeFileHandle)m_freeIndex;
		m_freeIndex = (i64)handle;
	}


	bool Read(fileHandle handle, void* buffer, size_t size, Function<void()> callback) override
	{
		File& file = m_files[(size_t)handle];

		file.callback = callback;
	}


	bool Write(fileHandle handle, void* data, size_t size, Function<void()> callback) override
	{

	}

private:
	IAllocator& m_allocator;
	Array<File> m_files;
	i64 m_freeIndex;
};


FileSystem* FileSystem::Create(IAllocator& allocator)
{
	return NEW_OBJECT(allocator, FileSystemImpl)(allocator);
}


void FileSystem::Destroy(FileSystem* system, IAllocator& allocator)
{
	DELETE_OBJECT(allocator, system);
}


}