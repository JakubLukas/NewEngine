#include "shader_manager.h"
#include "core/file/file_system.h"
#include "core/asserts.h"
#include "core/logs.h"

#include "core/string.h"


namespace bgfx
{


int compileShader(int _argc, const char* _argv[]);
//typedef void(*UserErrorFn)(void*, const char*, va_list);
//void setShaderCErrorFunction(UserErrorFn fn, void* user_ptr);
//connect error logging of shaderc to my own //////////////////////////////////////////////////////////////////////

}


namespace Veng
{


template<typename ...Args>
int LogCompileError(void* stream, char const* format, Args... args)
{
	LogError(format, args...);
}

/*static void CompileErrorCallback(void*, const char* format, va_list args)
{
	LogError(format, args);
}*/


static bool CompileShader(const FileSystem& fileSystem, const Path& path, Path& outPath)
{
	StaticInputBuffer<512> inPath;
	inPath << path.path;

	StaticInputBuffer<512> outPathBuffer;
	outPathBuffer << "shaders/compiled/";
	fileSystem.CreateDirectory(Path(outPathBuffer.Cstr()));
	const char* namePtr = string::FindStr(path.path, "raw");
	ASSERT2(namePtr != nullptr, "Wrong shader path");
	namePtr += 4;//move by raw/
	const char* ptr = string::FindChar(namePtr, '/');
	while(ptr != nullptr)
	{
		ptr += 1;//move by slash char
		outPathBuffer.Add(namePtr, ptr - namePtr);
		fileSystem.CreateDirectory(Path(outPathBuffer.Cstr()));
		namePtr = ptr;
		ptr = string::FindChar(ptr, '/');
	}
	const char* extPtr = string::FindCharR(path.path, '.');
	ASSERT2(extPtr != nullptr, "Wrong shader path");
	outPathBuffer.Add(namePtr, extPtr - namePtr);
	extPtr++;

	bool vertexShader;
	if (string::Equal(extPtr, "vsr"))
	{
		vertexShader = true;
		outPathBuffer << ".vs";
	}
	else if (string::Equal(extPtr, "fsr"))
	{
		vertexShader = false;
		outPathBuffer << ".fs";
	}

	StaticInputBuffer<512> includeDir;
	includeDir << "shaders/";

	StaticInputBuffer<512> varyingPath;
	const char* lastSlash = string::FindCharR(path.path, '/');
	ASSERT2(lastSlash != nullptr, "Wrong shader path");
	varyingPath.Add(path.path, lastSlash - path.path + 1);
	varyingPath << "varying.def";

	const char* args[] = {
		"-f",
		inPath.Cstr(),
		"-o",
		outPathBuffer.Cstr(),
		"-i",
		includeDir.Cstr(),
		"--platform",
		"windows",
		"--varyingdef",
		varyingPath.Cstr(),
		"--type",
		(vertexShader) ? "vertex" : "fragment",
		"--profile",
		(vertexShader) ? "vs_4_0" : "ps_4_0"
	};

	//bgfx::setShaderCErrorFunction(CompileErrorCallback, nullptr);
	if (bgfx::compileShader(sizeof(args) / sizeof(args[0]), args) == EXIT_FAILURE)
	{
		LogError("Failed to compile %s -> %s", path.path, outPathBuffer.Cstr());
		return false;
	}

	outPath = Path(outPathBuffer.Cstr());
	return true;
}



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

	Path vOutPath;
	CompileShader(GetFileSystem(), Path(vsPath), vOutPath);
	resourceHandle vsHandle = m_depManager->LoadResource(ResourceType::Shader, ResourceType::ShaderInternal, vOutPath);
	shader->vsHandle = static_cast<shaderInternalHandle>(vsHandle);

	char fsPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadLine(fsPath, Path::MAX_LENGTH));

	Path fOutPath;
	CompileShader(GetFileSystem(), Path(fsPath), fOutPath);
	resourceHandle fsHandle = m_depManager->LoadResource(ResourceType::Shader, ResourceType::ShaderInternal, fOutPath);
	shader->fsHandle = static_cast<shaderInternalHandle>(fsHandle);
}


void ShaderManager::ChildResourceLoaded(resourceHandle childResource)
{
	shaderInternalHandle childHandle = static_cast<shaderInternalHandle>(childResource);

	for (auto& res : m_resources)
	{
		Shader* shader = static_cast<Shader*>(res.value);

		if (shader->vsHandle == childHandle)
			shader->vsLoaded = true;
		else if (shader->fsHandle == childHandle)
			shader->fsLoaded = true;

		if (shader->vsLoaded && shader->fsLoaded)
			FinalizeShader(shader);
	}
}


void ShaderManager::FinalizeShader(Shader* shader)
{
	const Resource* vsRes = m_depManager->GetResource(ResourceType::ShaderInternal, static_cast<resourceHandle>(shader->vsHandle));
	const Resource* fsRes = m_depManager->GetResource(ResourceType::ShaderInternal, static_cast<resourceHandle>(shader->fsHandle));
	const ShaderInternal* vs = static_cast<const ShaderInternal*>(vsRes);
	const ShaderInternal* fs = static_cast<const ShaderInternal*>(fsRes);
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