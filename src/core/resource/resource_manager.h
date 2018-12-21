#pragma once

#include "core/iallocator.h"
#include "core/containers/hash_map.h"
#include "core/file/file_system.h"

#include "resource.h"
#include "dependency_manager.h"


namespace Veng
{

class InputBlob;
class DependencyManager;


static u32 HashFileHandle(const fileHandle& key)
{
	return static_cast<u32>(key);
};


class ResourceManager
{
	friend class ResourceManagementImpl;

public:
	ResourceManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	virtual ~ResourceManager();

	virtual ResourceType GetType() const = 0;

	virtual const char* const * GetSupportedFileExt() const = 0;
	virtual size_t GetSupportedFileExtCount() const = 0;

protected:
	resourceHandle Load(const Path& path);
	void Unload(resourceHandle handle);
	void Reload(resourceHandle handle);

	inline Resource* GetResource(resourceHandle handle) const;
	inline resourceHandle GetResourceHandle(Resource* resource) const;

protected:
	virtual Resource* CreateResource() = 0;
	virtual void DestroyResource(Resource* resource) = 0;
	virtual void ReloadResource(Resource* resource) = 0;

	virtual void ResourceLoaded(resourceHandle handle, InputBlob& data) = 0;
	virtual void ChildResourceLoaded(resourceHandle handle, ResourceType type) { }

	const FileSystem& GetFileSystem() const { return m_fileSystem; }

private:
	void LoadResource(const Path& path, Resource* resource);
	void FileSystemCallback(fileHandle handle);

protected:
	IAllocator& m_allocator;
	HashMap<u32, Resource*> m_resources;
	DependencyManager* m_depManager;
	ResourceType m_type;

private:
	struct ResourceAsyncOp
	{
		void* buffer;
		size_t bufferSize;
		resourceHandle handle;
	};

private:
	FileSystem& m_fileSystem;
	HashMap<fileHandle, ResourceAsyncOp> m_asyncOps;
};


}