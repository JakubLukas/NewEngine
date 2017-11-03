#pragma once

#include "core/hash_map.h"


namespace Veng
{


struct Shader
{

};


class ShaderManager
{
public:
	ShaderManager(IAllocator& allocator)
		: m_allocator(allocator)
		, m_shaders(m_allocator)
	{
	}
	~ShaderManager()
	{}

private:
	IAllocator& m_allocator;
	HashMap<u32, Shader> m_shaders;
};


}