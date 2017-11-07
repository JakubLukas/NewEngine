#pragma once

#include "core/hash_map.h"

#include <bgfx/bgfx.h>


namespace Veng
{


struct Shader
{
	bgfx::ShaderHandle handle = BGFX_INVALID_HANDLE;
};


class ShaderManager final
{
public:
	ShaderManager(IAllocator& allocator);
	~ShaderManager();


	Shader* GetShader(const char* path);

private:
	IAllocator& m_allocator;
	HashMap<const char*, Shader> m_shaders;
};


}