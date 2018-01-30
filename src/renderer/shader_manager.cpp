#include "shader_manager.h"
#include "core/file/file_system.h"
#include "core/asserts.h"

#include "core/string.h"


namespace Veng
{


ShaderInternalManager::ShaderInternalManager(IAllocator& allocator, FileSystem& fileSystem)
	: ResourceManager(allocator, fileSystem)
{}


ShaderInternalManager::~ShaderInternalManager()
{
}


shaderInternalHandle ShaderInternalManager::Load(const Path& path)
{
	return static_cast<shaderInternalHandle>(LoadInternal(path));
}


void ShaderInternalManager::Unload(shaderInternalHandle handle)
{
	UnloadInternal(static_cast<resourceHandle>(handle));
}


void ShaderInternalManager::Reload(shaderInternalHandle handle)
{
	ReloadInternal(static_cast<resourceHandle>(handle));
}


const ShaderInternal* ShaderInternalManager::GetResource(shaderInternalHandle handle) const
{
	return static_cast<const ShaderInternal*>(ResourceManager::GetResource(static_cast<resourceHandle>(handle)));
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


bool ShaderInternalManager::ResourceLoaded(Resource* resource, InputBlob& data)
{
	ShaderInternal* shaderInt = static_cast<ShaderInternal*>(resource);

	const bgfx::Memory* mem = bgfx::alloc((u32)data.GetSize() + 1);
	memory::Copy(mem->data, data.GetData(), data.GetSize());
	mem->data[mem->size - 1] = '\0';

	shaderInt->handle = bgfx::createShader(mem);
	if (bgfx::isValid(shaderInt->handle))
	{
		bgfx::setName(shaderInt->handle, resource->GetPath().path);
		return true;
	}
	else
	{
		ASSERT(false);
		return false;
	}
}


//=============================================================================


ShaderManager::ShaderManager(IAllocator& allocator, FileSystem& fileSystem)
	: ResourceManager(allocator, fileSystem)
	, m_internalShaders(m_allocator, fileSystem)
{
	m_internalShaders.SetOwner(this);
}


ShaderManager::~ShaderManager()
{
}


shaderHandle ShaderManager::Load(const Path& path)
{
	return static_cast<shaderHandle>(LoadInternal(path));
}


void ShaderManager::Unload(shaderHandle handle)
{
	UnloadInternal(static_cast<resourceHandle>(handle));
}


void ShaderManager::Reload(shaderHandle handle)
{
	ReloadInternal(static_cast<resourceHandle>(handle));
}


const Shader* ShaderManager::GetResource(shaderHandle handle) const
{
	return static_cast<Shader*>(ResourceManager::GetResource(static_cast<resourceHandle>(handle)));
}


Resource* ShaderManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, Shader)();
}


void ShaderManager::DestroyResource(Resource* resource)
{
	Shader* shader = static_cast<Shader*>(resource);
	
	m_internalShaders.Unload(shader->vsHandle);
	m_internalShaders.Unload(shader->fsHandle);

	bgfx::destroy(shader->program.handle);

	DELETE_OBJECT(m_allocator, shader);
}


void ShaderManager::ReloadResource(Resource* resource)
{
}


bool ShaderManager::ResourceLoaded(Resource* resource, InputBlob& data)
{
	Shader* shader = static_cast<Shader*>(resource);

	char vsPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadString(vsPath, Path::MAX_LENGTH));
	shader->vsHandle = m_internalShaders.Load(Path(vsPath));
	m_internalShaders.AddDependency(resource, static_cast<resourceHandle>(shader->vsHandle));

	data.Trim();

	char fsPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadString(fsPath, Path::MAX_LENGTH));
	shader->fsHandle = m_internalShaders.Load(Path(fsPath));
	m_internalShaders.AddDependency(resource, static_cast<resourceHandle>(shader->fsHandle));

	return true;
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
	const ShaderInternal* vs = m_internalShaders.GetResource(shader->vsHandle);
	const ShaderInternal* fs = m_internalShaders.GetResource(shader->fsHandle);
	shader->program.handle = bgfx::createProgram(vs->handle, fs->handle, false);

	if (bgfx::isValid(shader->program.handle))
	{
		shader->SetState(Resource::State::Ready);
	}
	else
	{
		ASSERT(false);
	}
}


}