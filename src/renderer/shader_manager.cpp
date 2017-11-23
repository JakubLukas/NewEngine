#include "shader_manager.h"
#include "core/file/file_system.h"
#include "core/asserts.h"

#include "core/string.h"


namespace Veng
{


bool Shader::IsValid() const
{
	return (
		   bgfx::isValid(vertex.handle)
		&& bgfx::isValid(fragment.handle)
		&& bgfx::isValid(program.handle)
		);
}


static bgfx::ShaderHandle CreateShader(const char* path)
{
	static FS::FileMode mode{
		FS::FileMode::Access::Read,
		FS::FileMode::ShareMode::ShareRead,
		FS::FileMode::CreationDisposition::OpenExisting,
		FS::FileMode::FlagNone
	};

	FS::File file;
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
	m_shaders
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
	newProgramInt.handle = bgfx::createProgram(vertShaderInt.handle, fragShaderInt.handle, true /* destroy shaders when program is destroyed */);

	if(bgfx::isValid(newProgramInt.handle))
		return m_programs.Insert(key, newProgramInt);
	else
		ASSERT2(false, "Invalid handle");

	return nullptr;
}


}