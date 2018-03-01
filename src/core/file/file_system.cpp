#include "file_system.h"

#include "file.h"
#include "core/array.h"
#include "core/os/os_utils.h"


namespace Veng
{





struct File
{
	nativeFileHandle handle;
	FS::Operation* operation = nullptr;
	Function<void(fileHandle)> callback;
	size_t position = 0;
	size_t size = 0;
};


class FileSystemImpl : public FileSystem
{
public:
	FileSystemImpl(IAllocator& allocator)
		: m_allocator(allocator)
		, m_operations(m_allocator)
		, m_files(m_allocator)
		, m_freeIndex(-1)
	{
		ASSERT(FS::CreateAsyncHandle(m_asyncHandle));

		m_callback.Bind<FileSystemImpl, &FileSystemImpl::Callback>(this);

		os::GetCurrentDir(m_currentDir.path, Path::MAX_LENGTH);
	}


	~FileSystemImpl() override
	{
		ASSERT(FS::DestroyAsyncHandle(m_asyncHandle));
	}


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
			file = &m_files.Push();
			file->operation = &m_operations.Push();
		}

		if (FS::OpenFile(file->handle, m_asyncHandle, path, mode))
		{
			file->size = FS::GetFileSize(file->handle);
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
		file.handle = (nativeFileHandle)m_freeIndex;
		m_freeIndex = (i64)handle;
	}


	bool Read(fileHandle handle, void* buffer, size_t size, Function<void(fileHandle)> callback) override
	{
		File& file = m_files[(size_t)handle];
		file.callback = callback;
		if(FS::ReadFile(file.handle, file.operation, file.position, buffer, size))
		{
			return true;
		}
		else
		{
			ASSERT(false);
			return false;
		}
	}


	bool Write(fileHandle handle, void* data, size_t size, Function<void(fileHandle)> callback) override
	{
		File& file = m_files[(size_t)handle];
		file.callback = callback;

		if(FS::WriteFile(file.handle, file.operation, file.position, data, size))
		{
			return true;
		}
		else
		{
			ASSERT(false);
			return false;
		}
	}


	void SetPosition(fileHandle handle, MoveMethod method, size_t position) override
	{
		File& file = m_files[(size_t)handle];

		switch(method)
		{
			case MoveMethod::Begin:
				file.position = position;
				break;
			case MoveMethod::Current:
				file.position += position;
			case MoveMethod::End:
				file.position = file.size - position;
		}
		file.position = position;
	}


	size_t GetPosition(fileHandle handle) const override
	{
		const File& file = m_files[(size_t)handle];
		return file.position;
	}


	size_t GetSize(fileHandle handle) const override
	{
		const File& file = m_files[(size_t)handle];
		return file.size;
	}


	void Update(float deltaTime) override
	{
		FS::QueryChanges(m_asyncHandle, m_callback);
	}


	const Path& GetCurrentDir() const override
	{
		return m_currentDir;
	}


	virtual bool CreateDirectory(const Path& path) const override
	{
		return FS::CreateDir(path);
	}

	virtual bool RemoveDirectory(const Path& path) const override
	{
		return FS::RemoveDir(path);
	}


private:
	void Callback(nativeFileHandle handle, size_t bytesTransfered)
	{
		for(size_t i = 0; i < m_files.GetSize(); ++i)
		{
			File& file = m_files[i];
			if(file.handle == handle)
			{
				file.size = FS::GetFileSize(handle);
				file.position += bytesTransfered;
				file.callback((fileHandle)i);
				return;
			}
		}

		ASSERT2(false, "File with given handle doesn't exist");
	}

private:
	IAllocator& m_allocator;
	nativeAsyncHandle m_asyncHandle;
	Array<FS::Operation> m_operations;
	Array<File> m_files;
	Function<void(nativeFileHandle, size_t)> m_callback;
	i64 m_freeIndex;
	Path m_currentDir;
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