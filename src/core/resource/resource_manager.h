#pragma once

#include "core/allocators.h"
#include "core/array.h"
#include "core/hash_map.h"
#include "core/file/file_system.h"
#include "core/file/blob.h"
#include "resource.h"


namespace Veng
{


enum resourceHandle : u64 {};


template<>
struct HashCalculator<fileHandle>
{
	static u64 Get(const fileHandle& key)
	{
		return static_cast<u64>(key);
	}
};


class ResourceManager
{
private:
	struct ResourceLoadingTemp
	{
		void* buffer;
		size_t bufferSize;
		Resource* resource;
	};

	struct DependencyPair
	{
		resourceHandle parent;
		resourceHandle child;
	};

public:
	ResourceManager(IAllocator& allocator, FileSystem& fileSystem);
	virtual ~ResourceManager();

protected:
	resourceHandle LoadInternal(const Path& path);
	void UnloadInternal(resourceHandle handle);
	void ReloadInternal(resourceHandle handle);

protected:
	Resource* GetResource(resourceHandle handle) const;

	virtual Resource* CreateResource() = 0;
	virtual void DestroyResource(Resource* resource) = 0;
	virtual void ReloadResource(Resource* resource) = 0;

	virtual bool ResourceLoaded(Resource* resource, InputBlob& data) = 0;
	virtual void ChildResourceLoaded(resourceHandle childResource) { }

public:///////////////////////////////////////////////////////////////////////////////////
	void SetOwner(ResourceManager* parent) { m_owner = parent; }
	void AddDependency(Resource* parent, resourceHandle child);
	void RemoveDependency(resourceHandle parent, resourceHandle child);
	void RemoveAllDependencies(resourceHandle parent);

private:
	void LoadResource(const Path& path, Resource* resource);
	void FileSystemCallback(fileHandle handle);

protected:
	IAllocator& m_allocator;
private:
	FileSystem& m_fileSystem;
	HashMap<fileHandle, ResourceLoadingTemp> m_loadingTemps;
protected:
	HashMap<u32, Resource*> m_resources;
private:
	ResourceManager* m_owner = nullptr;
	Array<DependencyPair> m_dependencies;
};


}