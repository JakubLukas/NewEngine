#include "resource_manager.h"

#include "core/file/blob.h"


namespace Veng
{


ResourceManager::ResourceManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: m_allocator(allocator)
	, m_fileSystem(fileSystem)
	, m_asyncOps(m_allocator, &HashFileHandle)
	, m_resources(m_allocator, &HashU32)
	, m_depManager(depManager)
{
}


ResourceManager::~ResourceManager()
{
	for (auto& tmp : m_asyncOps)
		ASSERT2(false, "Async operation on file in process");
	for (auto& node : m_resources)
		ASSERT2(false, "Resource not released");
}


resourceHandle ResourceManager::Load(const Path& path)
{
	Resource** item;
	if (!m_resources.Find(path.GetHash(), item))
	{
		Resource* res = CreateResource();
		item = m_resources.Insert(path.GetHash(), res);
		LoadResource(path, *item);
	}

	(*item)->m_refCount++;

	return GetResourceHandle(*item);
}


void ResourceManager::Unload(resourceHandle handle)
{
	Resource* resource = GetResource(handle);
	Path::Hash hash = resource->m_path.GetHash();
	Resource** item;
	if (m_resources.Find(hash, item))
	{
		ASSERT(resource == *item);
		if (0 == --resource->m_refCount)
		{
			m_fileSystem.CloseFile((*item)->m_fileHandle);
			DestroyResource(*item);
			m_resources.Erase(hash);
		}
	}
	else
	{
		ASSERT2(false, "Resource with given handle doesn't exist");
	}
}


void ResourceManager::Reload(resourceHandle handle)
{
	Resource* resource = GetResource(handle);
	Path::Hash hash = resource->m_path.GetHash();
	Resource** item;
	if (m_resources.Find(hash, item))
	{
		ASSERT(resource == *item);
		ReloadResource(*item);
	}
	else
	{
		ASSERT2(false, "Resource with given handle doesn't exist");
	}
}


Resource* ResourceManager::GetResource(resourceHandle handle) const
{
	return reinterpret_cast<Resource*>(handle);
}


resourceHandle ResourceManager::GetResourceHandle(Resource* resource) const
{
	return static_cast<resourceHandle>(reinterpret_cast<u64>(resource));
}


void ResourceManager::LoadResource(const Path& path, Resource* resource)
{
	static const FileMode mode{
		FileMode::Access::Read,
		FileMode::ShareMode::ShareRead,
		FileMode::CreationDisposition::OpenExisting,
		FileMode::FlagNone
	};

	ResourceAsyncOp tmp;
	ASSERT(m_fileSystem.OpenFile(resource->m_fileHandle, path, mode));
	tmp.bufferSize = m_fileSystem.GetSize(resource->m_fileHandle);
	tmp.buffer = m_allocator.Allocate(tmp.bufferSize, alignof(char));
	tmp.handle = GetResourceHandle(resource);
	Function<void(fileHandle)> f;
	f.Bind<ResourceManager, &ResourceManager::FileSystemCallback>(this);
	ASSERT(m_fileSystem.Read(resource->m_fileHandle, tmp.buffer, tmp.bufferSize, f));

	resource->m_path = path;
	resource->m_state = Resource::State::Loading;

	m_asyncOps.Insert(resource->m_fileHandle, tmp);
}


void ResourceManager::FileSystemCallback(fileHandle handle)
{
	ResourceAsyncOp* tmp;
	if (m_asyncOps.Find(handle, tmp))
	{
		InputBlob blob(static_cast<char*>(tmp->buffer), tmp->bufferSize);

		ResourceLoaded(tmp->handle, blob);

		m_allocator.Deallocate(tmp->buffer);
		ASSERT(m_asyncOps.Erase(handle));
	}
	else
	{
		ASSERT2(false, "No ResourceAsyncOp exists for fileHandle returned by callback");
	}
}


}
