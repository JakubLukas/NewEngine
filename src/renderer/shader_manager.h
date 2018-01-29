#pragma once

#include "core/hash_map.h"

#include <bgfx/bgfx.h>




#include "core/resource/resource.h"
#include "core/resource/resource_manager.h"
#include "core/file/file_system.h"
#include "core/file/path.h"
#include "core/array.h"
#include "core/string.h"




namespace Veng
{


/*struct ShaderInternal
{
	bgfx::ShaderHandle handle = BGFX_INVALID_HANDLE;
};

struct ShaderProgramInternal
{
	bgfx::ProgramHandle handle = BGFX_INVALID_HANDLE;
};

struct Shader
{
	ShaderInternal vertex;
	ShaderInternal fragment;
	ShaderProgramInternal program;
	//Uniforms
	//TextureSlots

	bool IsValid() const;
};


struct ProgramHashPair
{
	ShaderInternal vertex;
	ShaderInternal fragment;

	bool operator==(const ProgramHashPair& other)
	{
		return vertex.handle.idx == other.vertex.handle.idx
			&& fragment.handle.idx == other.fragment.handle.idx;
	}
	bool operator!=(const ProgramHashPair& other)
	{
		return !(operator==(other));
	}
};

template<>
struct HashCalculator<ProgramHashPair>
{
	static u32 Get(const ProgramHashPair& key)
	{
		//TODO: just dummy value to test (internally in bgfx are handles 16b values)
		return (key.vertex.handle.idx << 16u) + key.fragment.handle.idx;
	}
};


class ShaderManager final
{
public:
	ShaderManager(IAllocator& allocator);
	~ShaderManager();


	Shader GetShader(const char* vertexPath, const char* fragmentPath);

private:
	ShaderInternal* GetShaderInt(const char* path);
	ShaderProgramInternal* GetProgramInt(
		const char* vertexPath, const char* fragmentPath,
		ShaderInternal vertShaderInt, ShaderInternal fragShaderInt
	);

	IAllocator& m_allocator;
	HashMap<const char*, ShaderInternal> m_shaders;
	HashMap<ProgramHashPair, ShaderProgramInternal> m_programs;
};*/





template<>
struct HashCalculator<fileHandle>
{
	static u64 Get(const fileHandle& key)
	{
		return static_cast<u64>(key);
	}
};



struct NewShader : public Resource
{
	char* fileContent;
};





/*struct NewShaderInternal : public Resource
{
	bgfx::ShaderHandle handle = BGFX_INVALID_HANDLE;
};


class NewShaderInternalTraits
{
	Resource* Create(IAllocator& allocator, FileSystem& fileSystem, const Path& path)
	{
		static FileMode mode{
			FileMode::Access::Read,
			FileMode::ShareMode::ShareRead,
			FileMode::CreationDisposition::OpenExisting,
			FileMode::FlagNone
		};

		fileHandle file;
		ASSERT(file.Open(path, mode));
		size_t fileSize = file.GetSize();
		const bgfx::Memory* mem = bgfx::alloc((u32)fileSize + 1);
		ASSERT(file.Read(mem->data, fileSize));
		mem->data[mem->size - 1] = '\0';
		file.Close();

		bgfx::ShaderHandle handle = bgfx::createShader(mem);

		bgfx::setName(handle, path);

		return handle;
	}

	void Destroy(IAllocator& m_allocator, Resource* item)
	{

	}

	void Reload(Resource* item)
	{

	}
};


struct NewShaderProgramInternal : public Resource
{
	bgfx::ProgramHandle handle = BGFX_INVALID_HANDLE;
};

class NewShaderProgramInternalTraits
{
	Resource* Create(IAllocator& allocator, FileSystem& fileSystem, const Path& path)
	{
		return nullptr;
	}

	void Destroy(IAllocator& m_allocator, Resource* item)
	{

	}

	void Reload(Resource* item)
	{

	}
};*/


struct NewShaderManagerResourceTemp
{
	fileHandle handle;
	void* buffer;
	size_t bufferSize;
	Resource* resource;
};


enum shaderHandle : u64 {};


class NewShaderManager final
{
public:
	NewShaderManager(IAllocator& allocator, FileSystem& fileSystem) : m_allocator(allocator), m_fileSystem(fileSystem), m_temps(m_allocator), m_resources(m_allocator) {}
	~NewShaderManager()
	{
		for(auto& tmp : m_temps)
			ASSERT2(false, "Manager still waits for some async file system operation");
		for(auto& node : m_resources)
			ASSERT2(false, "Resource not released");
	}

	shaderHandle Load(const Path& path)
	{
		u32 hash = Path::GetHash(path);
		Resource** item;
		if(!m_resources.Find(hash, item))
		{
			Resource* shader = CreateShader(path);
			item = m_resources.Insert(hash, shader);
		}

		(*item)->m_refCount++;

		return (shaderHandle)((u64)(*item));
	}

	void Unload(shaderHandle handle)
	{
		Resource* resource = (Resource*)(handle);
		u32 hash = Path::GetHash(resource->m_path);
		Resource** item;
		if(m_resources.Find(hash, item))
		{
			ASSERT(resource == *item);
			if(0 == --resource->m_refCount)
			{
				m_resources.Erase(hash);
				DestroyShader(resource);
			}
		}
		else
		{
			ASSERT2(false, "Resource with given handle doesn't exist");
		}
	}

private:
	Resource* CreateShader(const Path& path)
	{
		static FileMode mode{
			FileMode::Access::Read,
			FileMode::ShareMode::ShareRead,
			FileMode::CreationDisposition::OpenExisting,
			FileMode::FlagNone
		};

		NewShaderManagerResourceTemp tmp;
		m_fileSystem.OpenFile(tmp.handle, path, mode);
		tmp.bufferSize = m_fileSystem.GetSize(tmp.handle);
		tmp.buffer = m_allocator.Allocate(tmp.bufferSize, ALIGN_OF(char));
		Function<void(fileHandle)> f;
		f.Bind<NewShaderManager, &NewShaderManager::FileSystemCallback>(this);
		m_fileSystem.Read(tmp.handle, tmp.buffer, tmp.bufferSize, f);

		tmp.resource = NEW_OBJECT(m_allocator, NewShader)();
		tmp.resource->m_path = path;
		tmp.resource->m_state = Resource::State::Loading;

		m_temps.Insert(tmp.handle, tmp);

		return tmp.resource;
	}

	void DestroyShader(Resource* resource)
	{
		NewShader* shader = static_cast<NewShader*>(resource);
		m_allocator.Deallocate(shader->fileContent);
		DELETE_OBJECT(m_allocator, resource);
	}

	void FileSystemCallback(fileHandle handle)
	{
		NewShaderManagerResourceTemp* tmp;
		if(m_temps.Find(handle, tmp))
		{
			ASSERT(m_temps.Erase(handle));
			NewShader* shader = static_cast<NewShader*>(tmp->resource);
			shader->fileContent = (char*)m_allocator.Allocate(tmp->bufferSize + 1, ALIGN_OF(char));
			shader->fileContent[tmp->bufferSize] = '\0';
			string::Copy(shader->fileContent, (char*)tmp->buffer, tmp->bufferSize);
			m_allocator.Deallocate(tmp->buffer);
			shader->m_state = Resource::State::Ready;
		}
		else
		{
			ASSERT2(false, "No tmp structure for file handle returned by callback");
		}
	}

	IAllocator& m_allocator;
	FileSystem& m_fileSystem;
	HashMap<fileHandle, NewShaderManagerResourceTemp> m_temps;
	//ResourceManager<NewShaderInternal, NewShaderInternalTraits> m_shaders;
	//ResourceManager<NewShaderProgramInternal, NewShaderProgramInternalTraits> m_programs;
	HashMap<u32, Resource*> m_resources;
};


}