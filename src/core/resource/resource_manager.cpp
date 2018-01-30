#include "resource_manager.h"


namespace Veng
{


inline static resourceHandle GetManagerHandle(Resource* resource)
{
	return static_cast<resourceHandle>(reinterpret_cast<u64>(resource));
}


ResourceManager::ResourceManager(IAllocator& allocator, FileSystem& fileSystem)
	: m_allocator(allocator)
	, m_fileSystem(fileSystem)
	, m_loadingTemps(m_allocator)
	, m_resources(m_allocator)
	, m_dependencies(m_allocator)
{
}


ResourceManager::~ResourceManager()
{
	for (auto& tmp : m_loadingTemps)
		ASSERT2(false, "Loading of file in process");
	for (auto& node : m_resources)
		ASSERT2(false, "Resource not released");
}


resourceHandle ResourceManager::LoadInternal(const Path& path)
{
	Resource** item;
	if (!m_resources.Find(path.hash, item))
	{
		Resource* res = CreateResource();
		item = m_resources.Insert(path.hash, res);
		LoadResource(path, *item);
	}

	(*item)->m_refCount++;

	return GetManagerHandle(*item);
}


void ResourceManager::UnloadInternal(resourceHandle handle)
{
	Resource* resource = GetResource(handle);
	u32 hash = resource->m_path.hash;
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


void ResourceManager::ReloadInternal(resourceHandle handle)
{
	Resource* resource = GetResource(handle);
	u32 hash = resource->m_path.hash;
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


void ResourceManager::AddDependency(Resource* parent, resourceHandle child)
{
	ASSERT(m_owner != nullptr);
	/*DependencyPair& dep = m_dependencies.Push();
	dep.parent = GetManagerHandle(parent);
	dep.child = child;



	for (size_t i = 0; i < m_dependencies.GetSize(); ++i)
	{
		if (m_dependencies[i].parent == parent && m_dependencies[i].child == child)
		{
			m_dependencies.Erase((unsigned)i);
			return;
		}
	}*/

	for (auto& res : m_resources)
	{
		resourceHandle handle = GetManagerHandle(res.value);
		if (handle == child && res.value->m_state == Resource::State::Ready)
			m_owner->ChildResourceLoaded(handle);
	}
}


void ResourceManager::RemoveDependency(resourceHandle parent, resourceHandle child)
{
	/*for (size_t i = 0; i < m_dependencies.GetSize(); ++i)
	{
		if (m_dependencies[i].parent == parent && m_dependencies[i].child == child)
		{
			m_dependencies.Erase((unsigned)i);
			return;
		}
	}*/
}


void ResourceManager::RemoveAllDependencies(resourceHandle parent)
{
	/*for (size_t i = 0; i < m_dependencies.GetSize(); ++i)
	{
		if (m_dependencies[i].parent == parent)
		{
			m_dependencies.Erase((unsigned)i--);
		}
	}*/
}


void ResourceManager::LoadResource(const Path& path, Resource* resource)
{
	static const FileMode mode{
		FileMode::Access::Read,
		FileMode::ShareMode::ShareRead,
		FileMode::CreationDisposition::OpenExisting,
		FileMode::FlagNone
	};

	ResourceLoadingTemp tmp;
	ASSERT(m_fileSystem.OpenFile(resource->m_fileHandle, path, mode));
	tmp.bufferSize = m_fileSystem.GetSize(resource->m_fileHandle);
	tmp.buffer = m_allocator.Allocate(tmp.bufferSize, ALIGN_OF(char));
	Function<void(fileHandle)> f;
	f.Bind<ResourceManager, &ResourceManager::FileSystemCallback>(this);
	ASSERT(m_fileSystem.Read(resource->m_fileHandle, tmp.buffer, tmp.bufferSize, f));

	tmp.resource = resource;
	tmp.resource->m_path = path;
	tmp.resource->m_state = Resource::State::Loading;

	m_loadingTemps.Insert(resource->m_fileHandle, tmp);
}


void ResourceManager::FileSystemCallback(fileHandle handle)
{
	ResourceLoadingTemp* tmp;
	if (m_loadingTemps.Find(handle, tmp))
	{
		InputBlob blob(tmp->buffer, tmp->bufferSize);
		if (ResourceLoaded(tmp->resource, blob))
		{
			tmp->resource->m_state = Resource::State::Ready;
			if (m_owner != nullptr)
				m_owner->ChildResourceLoaded(GetManagerHandle(tmp->resource));
		}
		else
			tmp->resource->m_state = Resource::State::Failure;

		m_allocator.Deallocate(tmp->buffer);
		ASSERT(m_loadingTemps.Erase(handle));
	}
	else
	{
		ASSERT2(false, "No tmp structure for file handle returned by callback");
	}
}


}