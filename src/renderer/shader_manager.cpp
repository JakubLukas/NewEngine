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


static bool CompileShader(const Path& currentDir, const Path& path)
{
	StaticInputBuffer<512> inPath;
	inPath << currentDir.path << "/";
	inPath << path.path;

	StaticInputBuffer<512> outPath;
	outPath << currentDir.path << "/";
	outPath << "shaders/compiled/";
	const char* rawPtr = string::FindStr(path.path, "raw");
	ASSERT2(rawPtr != nullptr, "Wrong shader path");
	rawPtr += 4;
	const char* extPtr = string::FindCharR(path.path, '.');
	ASSERT2(extPtr != nullptr, "Wrong shader path");
	outPath.Add(rawPtr, extPtr - rawPtr);
	extPtr++;

	bool vertexShader;
	if (string::Equal(extPtr, "vsr"))
	{
		vertexShader = true;
		outPath << ".vs";
	}
	else if (string::Equal(extPtr, "fsr"))
	{
		vertexShader = false;
		outPath << ".fs";
	}

	StaticInputBuffer<512> includeDir;
	includeDir << currentDir.path << "/shaders/";

	StaticInputBuffer<512> varyingPath;
	varyingPath << currentDir.path << "/";
	const char* lastSlash = string::FindCharR(path.path, '/');
	ASSERT2(lastSlash != nullptr, "Wrong shader path");
	varyingPath.Add(path.path, lastSlash - path.path + 1);
	varyingPath << "varying.def";

	const char* args[] = {
		"-f",
		inPath.Cstr(),
		"-o",
		outPath.Cstr(),
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
	if (bgfx::compileShader(14, args) == EXIT_FAILURE)
	{
		LogError("Failed to compile %s -> %s", path.path, outPath.Cstr());
		return false;
	}
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

	CompileShader(GetFileSystem().GetCurrentDir(), Path(vsPath));
	StaticInputBuffer<512> vOutPath;//////////////////////////////////////////////////////////////////////
	vOutPath << "shaders/compiled/";
	const char* rawPtr = string::FindStr(vsPath, "raw");
	ASSERT2(rawPtr != nullptr, "Wrong shader path");
	rawPtr += 4;
	const char* extPtr = string::FindCharR(vsPath, '.');
	ASSERT2(extPtr != nullptr, "Wrong shader path");
	vOutPath.Add(rawPtr, extPtr - rawPtr);
	vOutPath << ".vs";//".vbin";
	resourceHandle vsHandle = m_depManager->LoadResource(ResourceType::Shader, ResourceType::ShaderInternal, Path(vOutPath.Cstr()));
	shader->vsHandle = static_cast<shaderInternalHandle>(vsHandle);

	char fsPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadLine(fsPath, Path::MAX_LENGTH));

	CompileShader(GetFileSystem().GetCurrentDir(), Path(fsPath));
	StaticInputBuffer<512> fOutPath;//////////////////////////////////////////////////////////////////////
	fOutPath << "shaders/compiled/";
	rawPtr = string::FindStr(fsPath, "raw");
	ASSERT2(rawPtr != nullptr, "Wrong shader path");
	rawPtr += 4;
	extPtr = string::FindCharR(fsPath, '.');
	ASSERT2(extPtr != nullptr, "Wrong shader path");
	fOutPath.Add(rawPtr, extPtr - rawPtr);
	fOutPath << ".fs";//".fbin";
	resourceHandle fsHandle = m_depManager->LoadResource(ResourceType::Shader, ResourceType::ShaderInternal, Path(fOutPath.Cstr()));
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