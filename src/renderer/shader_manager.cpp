#include "shader_manager.h"
#include "core/file/file_system.h"
#include "core/asserts.h"


namespace Veng
{


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

	return bgfx::createShader(mem);
}



ShaderManager::ShaderManager(IAllocator& allocator)
	: m_allocator(allocator)
	, m_shaders(m_allocator)
{
}

ShaderManager::~ShaderManager()
{}


Shader* ShaderManager::GetShader(const char* path)
{
	Shader* shader;
	if (m_shaders.Find(path, shader))
	{
		return shader;
	}
	else
	{
		Shader newShader;
		newShader.handle = CreateShader(path);
		return m_shaders.Insert(path, newShader);
	}
}


}