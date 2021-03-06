#include "shader_manager.h"

#include "core/memory.h"
#include "core/file/blob.h"
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

ResourceType ShaderInternal::RESOURCE_TYPE("shader_internal");


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



ShaderInternalManager::ShaderInternalManager(Allocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(ShaderInternal::RESOURCE_TYPE, allocator, fileSystem, depManager)
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

	m_depManager->ResourceLoaded(ShaderInternal::RESOURCE_TYPE, handle);
}


//=============================================================================

ResourceType Shader::RESOURCE_TYPE("shader");


ShaderManager::ShaderManager(Allocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(Shader::RESOURCE_TYPE, allocator, fileSystem, depManager)
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
	
	m_depManager->UnloadResource(ShaderInternal::RESOURCE_TYPE, static_cast<resourceHandle>(shader->vsHandle));
	m_depManager->UnloadResource(ShaderInternal::RESOURCE_TYPE, static_cast<resourceHandle>(shader->fsHandle));

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
	ASSERT(JsonParseError((char*)data.GetData(), &m_allocator, &parsedJson, errorBuffer));
	ASSERT(JsonIsObject(&parsedJson));

	JsonKeyValue* varyings = JsonObjectFind(&parsedJson, "varyings");
	if (varyings != nullptr)
	{
		ASSERT(JsonIsArray(&varyings->value));
		const JsonValue* val = JsonArrayCBegin(&varyings->value);
		for (size_t i = 0; i < JsonArrayCount(&varyings->value); ++i)
		{
			ASSERT(JsonIsString(val));
			shader->varyings |= GetShaderVaryingFromString(JsonGetString(val));
			val++;
		}
	}

	JsonKeyValue* uniforms = JsonObjectFind(&parsedJson, "uniforms");
	if (uniforms != nullptr)
	{
		ASSERT(JsonIsArray(&uniforms->value));
		const JsonValue* val = JsonArrayCBegin(&uniforms->value);
		for (size_t i = 0; i < JsonArrayCount(&uniforms->value); ++i)
		{
			ASSERT(JsonIsString(val));
			const char* uniformName = JsonGetString(val);
			size_t uniformNameLen = string::Length(uniformName);
			if (uniformNameLen > ShaderUniform::MAX_NAME_LENGTH)
			{
				Log(LogType::Error, "Uniform name is too long, clamped to %d chars\n", ShaderUniform::MAX_NAME_LENGTH);
				uniformNameLen = ShaderUniform::MAX_NAME_LENGTH;
			}
			string::Copy(shader->inputUniforms[shader->inputUniformCount++].name, uniformName, uniformNameLen);

			val++;
		}
	}

	JsonKeyValue* textures = JsonObjectFind(&parsedJson, "textures");
	if (textures != nullptr)
	{
		ASSERT(JsonIsArray(&textures->value));
		const JsonValue* val = JsonArrayCBegin(&textures->value);
		for (size_t i = 0; i < JsonArrayCount(&textures->value); ++i)
		{
			ASSERT(JsonIsString(val));
			const char* textureName = JsonGetString(val);
			size_t textureNameLen = string::Length(textureName);
			if (textureNameLen > ShaderUniform::MAX_NAME_LENGTH)
			{
				Log(LogType::Error, "Texture name is too long, clamped to %d chars\n", ShaderUniform::MAX_NAME_LENGTH);
				textureNameLen = ShaderUniform::MAX_NAME_LENGTH;
			}
			string::Copy(shader->inputTextures[shader->inputTextureCount++].name, textureName, textureNameLen);

			val++;
		}
	}

	JsonKeyValue* lights = JsonObjectFind(&parsedJson, "lights");
	if(lights != nullptr)
	{
		ASSERT(JsonIsObject(&lights->value));
		
		JsonKeyValue* directional = JsonObjectFind(&parsedJson, "directional");
		if(directional != nullptr)
		{
			ASSERT(JsonIsInt(&directional->value));
			shader->inputDirectionalLightsCount = (u8)JsonGetInt(&directional->value);
		}
		JsonKeyValue* point = JsonObjectFind(&parsedJson, "point");
		if(point != nullptr)
		{
			ASSERT(JsonIsInt(&point->value));
			shader->inputPointLightsCount = (u8)JsonGetInt(&point->value);
		}
	}

	JsonKeyValue* vShader = JsonObjectFind(&parsedJson, "vertexShader");
	ASSERT(vShader != nullptr && JsonIsString(&vShader->value));
	Path vsPath(JsonGetString(&vShader->value));
	
	JsonKeyValue* fShader = JsonObjectFind(&parsedJson, "fragmentShader");
	ASSERT(fShader != nullptr && JsonIsString(&fShader->value));
	Path fsPath(JsonGetString(&fShader->value));

	JsonDeinit(&parsedJson);


	LoadingOp op;
	op.shader = handle;

	Path vOutPath;
	ASSERT(CompileShader(GetFileSystem(), vsPath, vOutPath));
	shader->vsHandle = m_depManager->LoadResource(Shader::RESOURCE_TYPE, ShaderInternal::RESOURCE_TYPE, vOutPath);
	op.vsHandle = shader->vsHandle;
	Resource* vRes = GetResource(shader->vsHandle);
	if(vRes->GetState() != Resource::State::Ready && vRes->GetState() != Resource::State::Failure)
		op.shadersToLoad |= LoadingOp::Shader_Vertex;

	Path fOutPath;
	ASSERT(CompileShader(GetFileSystem(), fsPath, fOutPath));
	shader->fsHandle = m_depManager->LoadResource(Shader::RESOURCE_TYPE, ShaderInternal::RESOURCE_TYPE, fOutPath);
	op.fsHandle = shader->fsHandle;
	Resource* fRes = GetResource(shader->fsHandle);
	if(fRes->GetState() != Resource::State::Ready && fRes->GetState() != Resource::State::Failure)
		op.shadersToLoad |= LoadingOp::Shader_Fragment;

	if(!LoadingOpCompleted(op))
		m_loadingOp.PushBack(op);
	else
		FinalizeShader(shader);
}


void ShaderManager::ChildResourceLoaded(resourceHandle handle, ResourceType type)
{
	for(size_t i = 0; i < m_loadingOp.GetSize(); ++i)
	{
		LoadingOp& op = m_loadingOp[i];

		if(op.vsHandle == handle)
			op.shadersToLoad &= ~LoadingOp::Shader_Vertex;
		else if(op.fsHandle == handle)
			op.shadersToLoad &= ~LoadingOp::Shader_Fragment;


		if(LoadingOpCompleted(op))
		{
			Shader* shader = static_cast<Shader*>(GetResource(op.shader));
			FinalizeShader(shader);
			m_loadingOp.Erase(i--);
		}
	}
}


void ShaderManager::FinalizeShader(Shader* shader)
{
	const Resource* vsRes = m_depManager->GetResource(ShaderInternal::RESOURCE_TYPE, static_cast<resourceHandle>(shader->vsHandle));
	const Resource* fsRes = m_depManager->GetResource(ShaderInternal::RESOURCE_TYPE, static_cast<resourceHandle>(shader->fsHandle));
	const ShaderInternal* vs = static_cast<const ShaderInternal*>(vsRes);
	const ShaderInternal* fs = static_cast<const ShaderInternal*>(fsRes);
	
	shader->renderDataHandle = m_renderSystem->CreateShaderData(vs->renderDataHandle, fs->renderDataHandle);

	m_depManager->ResourceLoaded(Shader::RESOURCE_TYPE, GetResourceHandle(shader));
}


bool ShaderManager::LoadingOpCompleted(const LoadingOp& op)
{
	return (op.shadersToLoad == LoadingOp::Shader_None);
}


}
