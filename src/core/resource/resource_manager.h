#pragma once

#include "core/allocators.h"
#include "core/hash_map.h"
#include "core/file/file_system.h"
#include "resource.h"


namespace Veng
{


enum resourceHandle : u64 {};


class ResourceManager final
{
	//static_assert(std::is_base_of<Key, T>::value, "T must inherit from Key");

public:
	ResourceManager(IAllocator& allocator, FileSystem& fileSystem)
		: m_allocator(allocator)
		, m_fileSystem(fileSystem)
		, m_resources(allocator)
	{
	}
	~ResourceManager()
	{
	}

	resourceHandle Load(const Path& path)
	{
		u32 hash = Path::GetHash(path);
		Resource* item;
		if(!m_resources.Find(hash, item))
		{
			item = CreateResource(path);
			m_resources.Insert(hash, item);
		}

		item->m_refCount++;

		return static_cast<resourceHandle>(item);
	}

	void Unload(resourceHandle handle)
	{
		Resource* resource = static_cast<Resource*>(handle);
		u32 hash = Path::GetHash(resource.m_path);
		Resource* item;
		if(m_resources.Find(hash, item))
		{
			ASSERT(resource == item);
			if(0 == --item->m_refCount)
			{
				m_resources.Erase(hash);
				ResourceTypeTraits::DestroyResource(m_allocator, item);
			}
		}
		else
		{
			ASSERT2(false, "Resource with given handle doesn't exist");
		}
	}

	void Reload(resourceHandle handle)
	{
		Resource* resource = static_cast<Resource*>(handle);
		u32 hash = Path::GetHash(resource.m_path);
		Resource* item;
		if(m_resources.Find(hash, item))
		{
			ASSERT(resource == item);
			ResourceTypeTraits::ReloadResource(item);
		}
		else
		{
			ASSERT2(false, "Resource with given handle doesn't exist");
		}
	}

	ResourceType& GetResource(resourceHandle handle)
	{
		return *static_cast<Resource*>(handle);
	}

private:
	virtual Resource* CreateResource(const Path& path) = 0;

private:
	IAllocator& m_allocator;
	FileSystem& m_fileSystem;
	HashMap<u32, Resource*> m_resources;
};


}