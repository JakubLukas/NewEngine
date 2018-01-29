#include "shader_manager.h"
#include "core/file/file_system.h"
#include "core/asserts.h"

#include "core/string.h"


namespace Veng
{


/*bool Shader::IsValid() const
{
	return (
		   bgfx::isValid(vertex.handle)
		&& bgfx::isValid(fragment.handle)
		&& bgfx::isValid(program.handle)
		);
}


static bgfx::ShaderHandle CreateShader(const char* path)
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



ShaderManager::ShaderManager(IAllocator& allocator)
	: m_allocator(allocator)
	, m_shaders(m_allocator)
	, m_programs(m_allocator)
{
}

ShaderManager::~ShaderManager()
{
	for (const HashMap<ProgramHashPair, ShaderProgramInternal>::Node& program : m_programs)
	{
		bgfx::destroy(program.value.handle);
	}

	for (const HashMap<const char*, ShaderInternal>::Node& shader : m_shaders)
	{
		bgfx::destroy(shader.value.handle);
	}
}


Shader ShaderManager::GetShader(const char* vertexPath, const char* fragmentPath)
{
	Shader result;

	result.vertex = *GetShaderInt(vertexPath);
	result.fragment = *GetShaderInt(fragmentPath);
	result.program = *GetProgramInt(vertexPath, fragmentPath, result.vertex, result.fragment);

	return result;
}


ShaderInternal* ShaderManager::GetShaderInt(const char* path)
{
	ShaderInternal* shaderInt;
	if(m_shaders.Find(path, shaderInt))
		return shaderInt;

	ShaderInternal newShaderInt;
	newShaderInt.handle = CreateShader(path);

	if(bgfx::isValid(newShaderInt.handle))
		return m_shaders.Insert(path, newShaderInt);
	else
		ASSERT2(false, "Invalid handle");

	return nullptr;
}


ShaderProgramInternal* ShaderManager::GetProgramInt(
	const char* vertexPath, const char* fragmentPath,
	ShaderInternal vertShaderInt, ShaderInternal fragShaderInt)
{
	ProgramHashPair key = { vertShaderInt, fragShaderInt };

	ShaderProgramInternal* programInt;
	if(m_programs.Find(key, programInt))
		return programInt;
	
	ShaderProgramInternal newProgramInt;
	newProgramInt.handle = bgfx::createProgram(vertShaderInt.handle, fragShaderInt.handle, false);

	if(bgfx::isValid(newProgramInt.handle))
		return m_programs.Insert(key, newProgramInt);
	else
		ASSERT2(false, "Invalid handle");

	return nullptr;
}*/





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


ShaderInternal* ShaderInternalManager::GetResource(shaderInternalHandle handle)
{
	return static_cast<ShaderInternal*>(ResourceManager::GetResource(static_cast<resourceHandle>(handle)));
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


bool ShaderInternalManager::ResourceLoaded(Resource* resource, void* buffer, size_t bufferSize)
{
	ShaderInternal* shaderInt = static_cast<ShaderInternal*>(resource);

	const bgfx::Memory* mem = bgfx::alloc((u32)bufferSize + 1);
	memory::Copy(mem->data, buffer, bufferSize);
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


Shader* ShaderManager::GetResource(shaderHandle handle)
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

	DELETE_OBJECT(m_allocator, resource);
}


void ShaderManager::ReloadResource(Resource* resource)
{
}


bool ShaderManager::ResourceLoaded(Resource* resource, void* const buffer, size_t bufferSize)
{
	Shader* shader = static_cast<Shader*>(resource);

	char* filePtr = (char*)buffer;
	char* begin = filePtr;
	size_t size = 0;
	for (size_t i = 0; i < bufferSize; ++i)
	{
		if (filePtr[i] != '\0' && filePtr[i] != '\r' && filePtr[i] != '\n')
		{
			size++;
		}
		else
		{
			if (size != 0)
			{
				shader->vsHandle = m_internalShaders.Load(Path(begin, size));
				m_internalShaders.AddDependency(resource, static_cast<resourceHandle>(shader->vsHandle));
			}
			size = 0;
			begin = filePtr + i + 1;
		}
	}

	ASSERT(size != 0);

	shader->fsHandle = m_internalShaders.Load(Path(begin, size));
	m_internalShaders.AddDependency(resource, static_cast<resourceHandle>(shader->fsHandle));

	return true;
}


bool ShaderManager::ChildResourceLoaded(resourceHandle childResource)
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

	return true;
}


bool ShaderManager::FinalizeShader(Shader* shader)
{
	ShaderInternal* vs = m_internalShaders.GetResource(shader->vsHandle);
	ShaderInternal* fs = m_internalShaders.GetResource(shader->fsHandle);
	shader->program.handle = bgfx::createProgram(vs->handle, fs->handle, false);

	if (bgfx::isValid(shader->program.handle))
	{
		SetResourceState(shader, Resource::State::Ready);
		return true;
	}
	else
	{
		ASSERT(false);
		return false;
	}
}


}