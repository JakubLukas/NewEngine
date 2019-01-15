#include "shader_manager.h"

#include "core/memory.h"
#include "core/file/blob.h"
#include "core/file/clob.h"
#include "core/file/file_system.h"
#include "core/asserts.h"
#include "core/logs.h"
#include "core/os/os_utils.h"

#include "core/string.h"

#include "../renderer.h"

#include "core/parsing/json.h"


namespace Veng
{

const char* const SHADER_VERT_FILE_EXT = ".vsr";
const char* const SHADER_FRAG_FILE_EXT = ".fsr";
const char* const SHADER_VERT_FILE_EXT_RET = ".vs";
const char* const SHADER_FRAG_FILE_EXT_RET = ".fs";


static ShaderVaryingBits GetShaderVaryingFromString(const char* str)
{
	if (string::Compare(str, "position"))
		return SV_POSITION_BIT;
	else if (string::Compare(str, "color0"))
		return SV_COLOR0_BIT;
	else if (string::Compare(str, "texcoord0"))
		return SV_TEXCOORDS0_BIT;
	else if (string::Compare(str, "texcoord1"))
		return SV_TEXCOORDS1_BIT;
	else if (string::Compare(str, "normal"))
		return SV_NORMAL_BIT;
	else if (string::Compare(str, "tangent"))
		return SV_TANGENT_BIT;
	else if (string::Compare(str, "binormal"))
		return SV_BINORMAL_BIT;
	else
		return SV_NONE;
}

static ShaderUniformBits GetShaderUniformFromString(const char* str)
{
	return SU_NONE;
}

static ShaderTextureBits GetShaderTextureFromString(const char* str)
{
	if (string::Compare(str, "diffuse"))
		return ST_DIFF_TEXTURE_BIT;
	else if (string::Compare(str, "normal"))
		return ST_NORM_TEXTURE_BIT;
	else
		return ST_NONE;
}


static bool CompileShader(const FileSystem& fileSystem, const Path& path, Path& outPath)
{
	static const char* IN_DIR = "raw";
	static const char* ROOT_PATH = "shaders/";
	static const char* OUT_PATH = "compiled/";
	static const char* COMMON_PATH = "common/";

	StaticInputBuffer<512> inPath;
	inPath << path.GetPath();

	StaticInputBuffer<512> outPathBuffer;
	outPathBuffer << ROOT_PATH << OUT_PATH;
	fileSystem.CreateDirectory(Path(outPathBuffer.Cstr()));
	const char* namePtr = string::FindStr(path.GetPath(), IN_DIR);
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
	const char* extPtr = string::FindCharR(path.GetPath(), '.');
	ASSERT2(extPtr != nullptr, "Wrong shader path");
	outPathBuffer.Add(namePtr, extPtr - namePtr);
	extPtr++;

	bool vertexShader;
	if (string::Equal(extPtr, SHADER_VERT_FILE_EXT + 1))
	{
		vertexShader = true;
		outPathBuffer << SHADER_VERT_FILE_EXT_RET;
	}
	else if (string::Equal(extPtr, SHADER_FRAG_FILE_EXT + 1))
	{
		vertexShader = false;
		outPathBuffer << SHADER_FRAG_FILE_EXT_RET;
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



ShaderInternalManager::ShaderInternalManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
{}


ShaderInternalManager::~ShaderInternalManager()
{
}


const char* const * ShaderInternalManager::GetSupportedFileExt() const
{
	static const char* buffer[] = { "vs", "fs" };
	return buffer;
}

size_t ShaderInternalManager::GetSupportedFileExtCount() const
{
	return 2;
}


void ShaderInternalManager::SetRenderSystem(RenderSystem* renderSystem)
{
	m_renderSystem = renderSystem;
}


Resource* ShaderInternalManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, ShaderInternal)();
}


void ShaderInternalManager::DestroyResource(Resource* resource)
{
	ShaderInternal* shaderInt = static_cast<ShaderInternal*>(resource);

	m_renderSystem->DestroyShaderInternalData(shaderInt->renderDataHandle);

	DELETE_OBJECT(m_allocator, shaderInt);
}


void ShaderInternalManager::ReloadResource(Resource* resource)
{
	ASSERT2(false, "Not implemented yet");
}


void ShaderInternalManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	ShaderInternal* shaderInt = static_cast<ShaderInternal*>(GetResource(handle));

	shaderInt->renderDataHandle = m_renderSystem->CreateShaderInternalData(data);

	m_depManager->ResourceLoaded(ResourceType::ShaderInternal, handle);
}


//=============================================================================


ShaderManager::ShaderManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
	, m_loadingOp(allocator)
{

}


ShaderManager::~ShaderManager()
{
}


const char* const * ShaderManager::GetSupportedFileExt() const
{
	static const char* buffer[] = { "shader" };
	return buffer;
}

size_t ShaderManager::GetSupportedFileExtCount() const
{
	return 1;
}


void ShaderManager::SetRenderSystem(RenderSystem* renderSystem)
{
	m_renderSystem = renderSystem;
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

	m_renderSystem->DestroyShaderData(shader->renderDataHandle);

	DELETE_OBJECT(m_allocator, shader);
}


void ShaderManager::ReloadResource(Resource* resource)
{
	ASSERT2(false, "Not implemented yet");
}


void ShaderManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Shader* shader = static_cast<Shader*>(ResourceManager::GetResource(handle));

	char errorBuffer[64] = { 0 };
	JsonValue parsedJson;
	ASSERT(JsonParse((char*)data.GetData(), &m_allocator, &parsedJson, errorBuffer));
	ASSERT(JsonIsObject(&parsedJson));

	JsonKeyValue* varyings = JsonObjectFind(&parsedJson, "varyings");
	if (varyings != nullptr)
	{
		ASSERT(JsonIsArray(&varyings->value));
		for (size_t i = 0; i < JsonArrayCount(&varyings->value); ++i)
		{
			const JsonValue* val = JsonArrayCBegin(&varyings->value);
			ASSERT(JsonIsString(val));
			shader->varyings |= GetShaderVaryingFromString(JsonGetString(val));
		}
	}

	JsonKeyValue* uniforms = JsonObjectFind(&parsedJson, "uniforms");
	if (uniforms != nullptr)
	{
		ASSERT(JsonIsArray(&uniforms->value));
		for (size_t i = 0; i < JsonArrayCount(&uniforms->value); ++i)
		{
			const JsonValue* val = JsonArrayCBegin(&uniforms->value);
			ASSERT(JsonIsString(val));
			shader->uniforms |= GetShaderUniformFromString(JsonGetString(val));
		}
	}

	JsonKeyValue* textures = JsonObjectFind(&parsedJson, "textures");
	if (textures != nullptr)
	{
		ASSERT(JsonIsArray(&textures->value));
		for (size_t i = 0; i < JsonArrayCount(&textures->value); ++i)
		{
			const JsonValue* val = JsonArrayCBegin(&textures->value);
			ASSERT(JsonIsString(val));
			shader->textures |= GetShaderTextureFromString(JsonGetString(val));
		}
	}

	JsonKeyValue* vShader = JsonObjectFind(&parsedJson, "vertexShader");
	ASSERT(vShader != nullptr && JsonIsString(&vShader->value));
	Path vsPath(JsonGetString(&vShader->value));
	
	JsonKeyValue* fShader = JsonObjectFind(&parsedJson, "fragmentShader");
	ASSERT(fShader != nullptr && JsonIsString(&fShader->value));
	Path fsPath(JsonGetString(&fShader->value));

	JsonDeinit(&parsedJson);


	LoadingOp& op = m_loadingOp.PushBack();
	op.shader = handle;

	Path vOutPath;
	ASSERT(CompileShader(GetFileSystem(), vsPath, vOutPath));
	shader->vsHandle = m_depManager->LoadResource(ResourceType::Shader, ResourceType::ShaderInternal, vOutPath);
	op.vsHandle = shader->vsHandle;

	Path fOutPath;
	ASSERT(CompileShader(GetFileSystem(), fsPath, fOutPath));
	shader->fsHandle = m_depManager->LoadResource(ResourceType::Shader, ResourceType::ShaderInternal, fOutPath);
	op.fsHandle = shader->fsHandle;
}


void ShaderManager::ChildResourceLoaded(resourceHandle handle, ResourceType type)
{
	for(size_t i = 0; i < m_loadingOp.GetSize(); ++i)
	{
		LoadingOp& op = m_loadingOp[i];

		if(op.vsHandle == handle)
			op.shadersIntLoaded |= 1;
		else if(op.fsHandle == handle)
			op.shadersIntLoaded |= 1 << 1;


		if(op.shadersIntLoaded == 3)
		{
			Shader* shader = static_cast<Shader*>(GetResource(op.shader));
			FinalizeShader(shader);
			m_loadingOp.Erase(i--);
		}
	}
}


void ShaderManager::FinalizeShader(Shader* shader)
{
	const Resource* vsRes = m_depManager->GetResource(ResourceType::ShaderInternal, static_cast<resourceHandle>(shader->vsHandle));
	const Resource* fsRes = m_depManager->GetResource(ResourceType::ShaderInternal, static_cast<resourceHandle>(shader->fsHandle));
	const ShaderInternal* vs = static_cast<const ShaderInternal*>(vsRes);
	const ShaderInternal* fs = static_cast<const ShaderInternal*>(fsRes);
	
	shader->renderDataHandle = m_renderSystem->CreateShaderData(vs->renderDataHandle, fs->renderDataHandle);

	m_depManager->ResourceLoaded(ResourceType::Shader, GetResourceHandle(shader));
}


}