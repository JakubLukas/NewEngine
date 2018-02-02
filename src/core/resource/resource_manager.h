#pragma once

#include "core/allocators.h"
#include "core/array.h"
#include "core/hash_map.h"
#include "core/file/file_system.h"
#include "core/file/blob.h"
#include "resource.h"

#include "dependency_manager.h"


namespace Veng
{


class ResourceManager
{
	friend class DependencyManager;

public:
	ResourceManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	virtual ~ResourceManager();

protected:
	template<class HandleType>
	HandleType Load(const Path& path)
	{
		return static_cast<HandleType>(LoadInternal(path));
	}

	template<class HandleType>
	void Unload(HandleType handle)
	{
		UnloadInternal(static_cast<resourceHandle>(handle));
	}

	template<class HandleType>
	void Reload(HandleType handle)
	{
		ReloadInternal(static_cast<resourceHandle>(handle));
	}

protected:
	template<class ResType, class HandleType>
	ResType* GetResource(HandleType handle) const
	{
		return static_cast<ResType*>(GetResourceInternal(static_cast<resourceHandle>(handle)));
	}

	template<class HandleType>
	HandleType GetResourceHandle(Resource* resource) const
	{
		return static_cast<resourceHandle>(reinterpret_cast<u64>(resource));
	}

	virtual Resource* CreateResource() = 0;
	virtual void DestroyResource(Resource* resource) = 0;
	virtual void ReloadResource(Resource* resource) = 0;

	virtual void ResourceLoaded(resourceHandle handle, InputBlob& data) = 0;
	virtual void ChildResourceLoaded(resourceHandle childResource) { }

private:
	resourceHandle LoadInternal(const Path& path);
	void UnloadInternal(resourceHandle handle);
	void ReloadInternal(resourceHandle handle);

	Resource* GetResourceInternal(resourceHandle handle) const;

	void LoadResource(const Path& path, Resource* resource);
	void FileSystemCallback(fileHandle handle);

protected:
	IAllocator& m_allocator;
	HashMap<u32, Resource*> m_resources;
	DependencyManager* m_depManager;


private:
	struct ResourceLoadingTemp
	{
		void* buffer;
		size_t bufferSize;
		resourceHandle handle;
	};

private:
	FileSystem& m_fileSystem;
	HashMap<fileHandle, ResourceLoadingTemp> m_loadingTemps;
};


}