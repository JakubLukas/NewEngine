#include "shader_manager.h"

#include "core/memory.h"
#include "core/file/blob.h"
#include "core/file/clob.h"
#include "core/file/file_system.h"
#include "core/asserts.h"
#include "core/logs.h"
#include "core/os/os_utils.h"

#include "core/string.h"


namespace Veng
{


static bool CompileShader(const FileSystem& fileSystem, const Path& path, Path& outPath)
{
	static const char* IN_DIR = "raw";
	static const char* ROOT_PATH = "shaders/";
	static const char* OUT_PATH = "compiled/";
	static const char* COMMON_PATH = "common/";
	static const char* IN_VERTEX_EXT = "vsr";
	static const char* IN_FRAGMENT_EXT = "fsr";
	static const char* OUT_VERTEX_EXT = "vs";
	static const char* OUT_FRAGMENT_EXT = "fs";

	StaticInputBuffer<512> inPath;
	inPath << path.path;

	StaticInputBuffer<512> outPathBuffer;
	outPathBuffer << ROOT_PATH << OUT_PATH;
	fileSystem.CreateDirectory(Path(outPathBuffer.Cstr()));
	const char* namePtr = string::FindStr(path.path, IN_DIR);
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
	if (string::Equal(extPtr, IN_VERTEX_EXT))
	{
		vertexShader = true;
		outPathBuffer << "." << OUT_VERTEX_EXT;
	}
	else if (string::Equal(extPtr, IN_FRAGMENT_EXT))
	{
		vertexShader = false;
		outPathBuffer << "." << OUT_FRAGMENT_EXT;
	}

	StaticInputBuffer<512> includeDir;
	includeDir << ROOT_PATH << COMMON_PATH;

	StaticInputBuffer<512> varyingPath;
	varyingPath << ROOT_PATH << COMMON_PATH << "varying.def";

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
	StaticInputBuffer<32000> cmdLine;
	cmdLine << "-f \"" << inPath.Cstr() << "\"";
	cmdLine << " -o \"" << outPathBuffer.Cstr() << "\"";
	cmdLine << " -i \"" << includeDir.Cstr() << "\"";
	cmdLine << " --platform " << "windows";
	cmdLine << " --varyingdef \"" << varyingPath.Cstr() << "\"";
	cmdLine << " --type " << ((vertexShader) ? "vertex" : "fragment");
	cmdLine << " --profile " << ((vertexShader) ? "vs_4_0" : "ps_4_0");

	os::CallProcess("./external/bgfx/shadercDebug.exe", cmdLine.Data());

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
	ASSERT2(false, "Not implemented yet");
}


void ShaderInternalManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	ShaderInternal* shaderInt = static_cast<ShaderInternal*>(ResourceManager::GetResource(handle));
	InputClob dataText(data);

	const bgfx::Memory* mem = bgfx::alloc((u32)dataText.GetSize() + 1);
	memory::Copy(mem->data, dataText.GetData(), dataText.GetSize());
	mem->data[mem->size - 1] = '\0';

	shaderInt->handle = bgfx::createShader(mem);
	if (bgfx::isValid(shaderInt->handle))
	{
		bgfx::setName(shaderInt->handle, shaderInt->GetPath().path);
		shaderInt->SetState(Resource::State::Ready);
	}
	else
	{
		ASSERT2(false, "Shader was not created");
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
	ASSERT2(false, "Not implemented yet");
}


void ShaderManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Shader* shader = static_cast<Shader*>(ResourceManager::GetResource(handle));
	InputClob dataText(data);

	char vsPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(dataText.ReadLine(vsPath, Path::MAX_LENGTH));

	Path vOutPath;
	ASSERT(CompileShader(GetFileSystem(), Path(vsPath), vOutPath));
	resourceHandle vsHandle = m_depManager->LoadResource(ResourceType::Shader, ResourceType::ShaderInternal, vOutPath);
	shader->vsHandle = static_cast<shaderInternalHandle>(vsHandle);

	char fsPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(dataText.ReadLine(fsPath, Path::MAX_LENGTH));

	Path fOutPath;
	ASSERT(CompileShader(GetFileSystem(), Path(fsPath), fOutPath));
	resourceHandle fsHandle = m_depManager->LoadResource(ResourceType::Shader, ResourceType::ShaderInternal, fOutPath);
	shader->fsHandle = static_cast<shaderInternalHandle>(fsHandle);
}


void ShaderManager::ChildResourceLoaded(resourceHandle handle, ResourceType type)
{
	shaderInternalHandle childHandle = static_cast<shaderInternalHandle>(handle);

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