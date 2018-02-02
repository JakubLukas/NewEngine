#include "shader_manager.h"
#include "core/file/file_system.h"
#include "core/asserts.h"

#include "core/string.h"


namespace Veng
{


ShaderInternalManager::ShaderInternalManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
{}


ShaderInternalManager::~ShaderInternalManager()
{
}


shaderInternalHandle ShaderInternalManager::Load(const Path& path)
{
	return ResourceManager::Load<shaderInternalHandle>(path);
}


void ShaderInternalManager::Unload(shaderInternalHandle handle)
{
	ResourceManager::Unload(handle);
}


void ShaderInternalManager::Reload(shaderInternalHandle handle)
{
	ResourceManager::Reload(handle);
}


const ShaderInternal* ShaderInternalManager::GetResource(shaderInternalHandle handle) const
{
	return ResourceManager::GetResource<ShaderInternal>(handle);
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
	ShaderInternal* shaderInt = ResourceManager::GetResource<ShaderInternal>(handle);

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


ShaderManager::ShaderManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
{

}


ShaderManager::~ShaderManager()
{
}


shaderHandle ShaderManager::Load(const Path& path)
{
	return ResourceManager::Load<shaderHandle>(path);
}


void ShaderManager::Unload(shaderHandle handle)
{
	ResourceManager::Unload(handle);
}


void ShaderManager::Reload(shaderHandle handle)
{
	ResourceManager::Reload(handle);
}


const Shader* ShaderManager::GetResource(shaderHandle handle) const
{
	return ResourceManager::GetResource<Shader>(handle);
}


Resource* ShaderManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, Shader)();
}


void ShaderManager::DestroyResource(Resource* resource)
{
	Shader* shader = static_cast<Shader*>(resource);
	
	m_depManager->UnloadResource(ResourceType::ShaderInternal, shader->vsHandle);
	m_depManager->UnloadResource(ResourceType::ShaderInternal, shader->fsHandle);

	bgfx::destroy(shader->program.handle);

	DELETE_OBJECT(m_allocator, shader);
}


void ShaderManager::ReloadResource(Resource* resource)
{
}


void ShaderManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Shader* shader = ResourceManager::GetResource<Shader>(handle);

	char vsPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadLine(vsPath, Path::MAX_LENGTH));
	shader->vsHandle = m_depManager->LoadResource<shaderInternalHandle>(ResourceType::Shader, ResourceType::ShaderInternal, Path(vsPath));

	char fsPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadLine(fsPath, Path::MAX_LENGTH));
	shader->fsHandle = m_depManager->LoadResource<shaderInternalHandle>(ResourceType::Shader, ResourceType::ShaderInternal, Path(fsPath));
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
	const ShaderInternal* vs = m_depManager->GetResource<ShaderInternal>(ResourceType::ShaderInternal, shader->vsHandle);
	const ShaderInternal* fs = m_depManager->GetResource<ShaderInternal>(ResourceType::ShaderInternal, shader->fsHandle);
	shader->program.handle = bgfx::createProgram(vs->handle, fs->handle, false);

	if (bgfx::isValid(shader->program.handle))
	{
		shader->SetState(Resource::State::Ready);
	}
	else
	{
		ASSERT(false);/////failed to load resource
	}
	m_depManager->ResourceLoaded(ResourceType::Shader, GetResourceHandle<resourceHandle>(shader));
}


}