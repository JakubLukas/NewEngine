#include "shader_manager.h"
#include "core/file/file_system.h"
#include "core/asserts.h"

#include "core/string.h"


namespace Veng
{


inline static shaderInternalHandle GenericToShaderIntHandle(resourceHandle handle)
{
	return static_cast<shaderInternalHandle>(handle);
}

inline static resourceHandle ShaderIntToGenericHandle(shaderInternalHandle handle)
{
	return static_cast<resourceHandle>(handle);
}


ShaderInternalManager::ShaderInternalManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
{}


ShaderInternalManager::~ShaderInternalManager()
{
}


shaderInternalHandle ShaderInternalManager::Load(const Path& path)
{
	return GenericToShaderIntHandle(ResourceManager::Load(path));
}


void ShaderInternalManager::Unload(shaderInternalHandle handle)
{
	ResourceManager::Unload(ShaderIntToGenericHandle(handle));
}


void ShaderInternalManager::Reload(shaderInternalHandle handle)
{
	ResourceManager::Reload(ShaderIntToGenericHandle(handle));
}


const ShaderInternal* ShaderInternalManager::GetResource(shaderInternalHandle handle) const
{
	return static_cast<const ShaderInternal*>(ResourceManager::GetResource(ShaderIntToGenericHandle(handle)));
}


Resource* ShaderInternalManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, ShaderInternal)();
}


void ShaderInternalManager::DestroyResource(Resource* resource)
{
	ShaderInternal* shaderInt = static_cast<ShaderInternal*>(resource);

	bgfx::destroy(shaderInt->handle);

	DELETE_OBJECT(m_allocator, shaderInt);
}


void ShaderInternalManager::ReloadResource(Resource* resource)
{
}


void ShaderInternalManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	ShaderInternal* shaderInt = static_cast<ShaderInternal*>(ResourceManager::GetResource(handle));

	const bgfx::Memory* mem = bgfx::alloc((u32)data.GetSize() + 1);
	memory::Copy(mem->data, data.GetData(), data.GetSize());
	mem->data[mem->size - 1] = '\0';

	shaderInt->handle = bgfx::createShader(mem);
	if (bgfx::isValid(shaderInt->handle))
	{
		bgfx::setName(shaderInt->handle, shaderInt->GetPath().path);
		shaderInt->SetState(Resource::State::Ready);
	}
	else
	{
		ASSERT(false);/////failed load of resource
	}
	m_depManager->ResourceLoaded(ResourceType::ShaderInternal, handle);
}


//=============================================================================


inline static shaderHandle GenericToShaderHandle(resourceHandle handle)
{
	return static_cast<shaderHandle>(handle);
}

inline static resourceHandle ShaderToGenericHandle(shaderHandle handle)
{
	return static_cast<resourceHandle>(handle);
}


ShaderManager::ShaderManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
{

}


ShaderManager::~ShaderManager()
{
}


shaderHandle ShaderManager::Load(const Path& path)
{
	return GenericToShaderHandle(ResourceManager::Load(path));
}


void ShaderManager::Unload(shaderHandle handle)
{
	ResourceManager::Unload(ShaderToGenericHandle(handle));
}


void ShaderManager::Reload(shaderHandle handle)
{
	ResourceManager::Reload(ShaderToGenericHandle(handle));
}


const Shader* ShaderManager::GetResource(shaderHandle handle) const
{
	return static_cast<const Shader*>(ResourceManager::GetResource(ShaderToGenericHandle(handle)));
}


Resource* ShaderManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, Shader)();
}


void ShaderManager::DestroyResource(Resource* resource)
{
	Shader* shader = static_cast<Shader*>(resource);
	
	m_depManager->UnloadResource(ResourceType::ShaderInternal, static_cast<resourceHandle>(shader->vsHandle));
	m_depManager->UnloadResource(ResourceType::ShaderInternal, static_cast<resourceHandle>(shader->fsHandle));

	bgfx::destroy(shader->program.handle);

	DELETE_OBJECT(m_allocator, shader);
}


void ShaderManager::ReloadResource(Resource* resource)
{
}


void ShaderManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Shader* shader = static_cast<Shader*>(ResourceManager::GetResource(handle));

	char vsPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadLine(vsPath, Path::MAX_LENGTH));
	shader->vsHandle = static_cast<shaderInternalHandle>(m_depManager->LoadResource(ResourceType::Shader, ResourceType::ShaderInternal, Path(vsPath)));

	char fsPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadLine(fsPath, Path::MAX_LENGTH));
	shader->fsHandle = static_cast<shaderInternalHandle>(m_depManager->LoadResource(ResourceType::Shader, ResourceType::ShaderInternal, Path(fsPath)));
}


void ShaderManager::ChildResourceLoaded(resourceHandle childResource)
{
	shaderInternalHandle childHandle = static_cast<shaderInternalHandle>(childResource);

	for (auto& res : m_resources)
	{
		Shader* shader = static_cast<Shader*>(res.value);
		if (shader->vsHandle == childHandle)
		{
			shader->vsLoaded = true;
			if (shader->fsLoaded)
			{
				FinalizeShader(shader);
			}
		}
		else if (shader->fsHandle == childHandle)
		{
			shader->fsLoaded = true;
			if (shader->vsLoaded)
				FinalizeShader(shader);
		}
	}
}


void ShaderManager::FinalizeShader(Shader* shader)
{
	const ShaderInternal* vs = static_cast<const ShaderInternal*>(m_depManager->GetResource(ResourceType::ShaderInternal, static_cast<resourceHandle>(shader->vsHandle)));
	const ShaderInternal* fs = static_cast<const ShaderInternal*>(m_depManager->GetResource(ResourceType::ShaderInternal, static_cast<resourceHandle>(shader->fsHandle)));
	shader->program.handle = bgfx::createProgram(vs->handle, fs->handle, false);

	if (bgfx::isValid(shader->program.handle))
	{
		shader->SetState(Resource::State::Ready);
	}
	else
	{
		ASSERT(false);/////failed to load resource
	}
	m_depManager->ResourceLoaded(ResourceType::Shader, GetResourceHandle(shader));
}


}