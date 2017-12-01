#pragma once

#include "core/hash_map.h"

#include <bgfx/bgfx.h>


namespace Veng
{


struct ShaderInternal
{
	bgfx::ShaderHandle handle = BGFX_INVALID_HANDLE;
};

struct ShaderProgramInternal
{
	bgfx::ProgramHandle handle = BGFX_INVALID_HANDLE;
};

struct Shader
{
	ShaderInternal vertex;
	ShaderInternal fragment;
	ShaderProgramInternal program;
	//Uniforms
	//TextureSlots

	bool IsValid() const;
};


struct ProgramHashPair
{
	ShaderInternal vertex;
	ShaderInternal fragment;

	bool operator==(const ProgramHashPair& other)
	{
		return vertex.handle.idx == other.vertex.handle.idx
			&& fragment.handle.idx == other.fragment.handle.idx;
	}
	bool operator!=(const ProgramHashPair& other)
	{
		return !(operator==(other));
	}
};

template<>
struct HashCalculator<ProgramHashPair>
{
	static u32 Get(const ProgramHashPair& key)
	{
		//TODO: just dummy value to test (internally in bgfx are handles 16b values)
		return (key.vertex.handle.idx << 16u) + key.fragment.handle.idx;
	}
};


class ShaderManager final
{
public:
	ShaderManager(IAllocator& allocator);
	~ShaderManager();


	Shader GetShader(const char* vertexPath, const char* fragmentPath);

private:
	ShaderInternal* GetShaderInt(const char* path);
	ShaderProgramInternal* GetProgramInt(
		const char* vertexPath, const char* fragmentPath,
		ShaderInternal vertShaderInt, ShaderInternal fragShaderInt
	);

	IAllocator& m_allocator;
	HashMap<const char*, ShaderInternal> m_shaders;
	HashMap<ProgramHashPair, ShaderProgramInternal> m_programs;
};


}