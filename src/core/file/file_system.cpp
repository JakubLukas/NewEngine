#include "file_system.h"

#include "file.h"
#include "core/array.h"
#include "core/object_pool.h"
#include "core/os/os_utils.h"


namespace Veng
{





struct File
{
	nativeFileHandle handle;
	FS::Operation operation;
	Function<void(fileHandle)> callback;
	size_t position = 0;
	size_t size = 0;
};


class FileSystemImpl : public FileSystem
{
public:
	FileSystemImpl(IAllocator& allocator)
		: m_allocator(allocator)
		, m_filePool(m_allocator)
		, m_files(m_allocator)
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
		File* file = m_filePool.GetObject();
		m_files.PushBack(file);

		if (FS::OpenFile(file->handle, m_asyncHandle, path, mode))
		{
			file->size = FS::GetFileSize(file->handle);
			handle = static_cast<fileHandle>(reinterpret_cast<u64>(file));
			return true;
		}
		else
		{
			return false;
		}
	}


	void CloseFile(fileHandle handle) override
	{
		File* file = reinterpret_cast<File*>(handle);

		ASSERT(FS::CloseFile(file->handle));

		file->callback.Clear();
		file->position = 0;
		file->handle = nullptr;

		ASSERT(m_files.Erase(file));
		m_filePool.ReturnObject(file);
	}


	bool Read(fileHandle handle, void* buffer, size_t size, Function<void(fileHandle)> callback) override
	{
		File* file = reinterpret_cast<File*>(handle);
		file->callback = callback;
		if(FS::ReadFile(file->handle, &file->operation, file->position, buffer, size))
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
		File* file = reinterpret_cast<File*>(handle);
		file->callback = callback;

		if(FS::WriteFile(file->handle, &file->operation, file->position, data, size))
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
		File* file = reinterpret_cast<File*>(handle);

		switch(method)
		{
			case MoveMethod::Begin:
				file->position = position;
				break;
			case MoveMethod::Current:
				file->position += position;
			case MoveMethod::End:
				file->position = file->size - position;
		}
		file->position = position;
	}


	size_t GetPosition(fileHandle handle) const override
	{
		const File* file = reinterpret_cast<File*>(handle);
		return file->position;
	}


	size_t GetSize(fileHandle handle) const override
	{
		const File* file = reinterpret_cast<File*>(handle);
		return file->size;
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
			File* file = m_files[i];
			if(file->handle == handle)
			{
				file->size = FS::GetFileSize(handle);
				file->position += bytesTransfered;
				file->callback(static_cast<fileHandle>(reinterpret_cast<u64>(file)));
				return;
			}
		}

		ASSERT2(false, "File with given handle doesn't exist");
	}

private:
	IAllocator& m_allocator;
	nativeAsyncHandle m_asyncHandle;
	ObjectPool<File> m_filePool;
	Array<File*> m_files;
	Function<void(nativeFileHandle, size_t)> m_callback;
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