#include "shader_inputs.h"

#include "core/asserts.h"
#include "core/string.h"


namespace Veng
{


ShaderVaryingBits GetShaderVaryingFromString(const char* str)
{
	if(string::Compare(str, "position"))
		return SV_POSITION_BIT;
	else if(string::Compare(str, "color0"))
		return SV_COLOR0_BIT;
	else if(string::Compare(str, "texcoord0"))
		return SV_TEXCOORDS0_BIT;
	else if(string::Compare(str, "texcoord1"))
		return SV_TEXCOORDS1_BIT;
	else if(string::Compare(str, "normal"))
		return SV_NORMAL_BIT;
	else if(string::Compare(str, "tangent"))
		return SV_TANGENT_BIT;
	else if(string::Compare(str, "binormal"))
		return SV_BINORMAL_BIT;
	else
	{
		ASSERT2(false, "Unrecognized value");
		return SV_NONE;
	}
}

const char* GetShaderVaryingName(ShaderVaryingBits value)
{
	if(value == SV_POSITION_BIT)
		return "position";
	else if(value == SV_COLOR0_BIT)
		return "color0";
	else if(value == SV_TEXCOORDS0_BIT)
		return "texcoord0";
	else if(value == SV_TEXCOORDS1_BIT)
		return "texcoord1";
	else if(value == SV_NORMAL_BIT)
		return "normal";
	else if(value == SV_TANGENT_BIT)
		return "tangent";
	else if(value == SV_BINORMAL_BIT)
		return "binormal";
	else
	{
		ASSERT2(false, "Unrecognized value");
		return "";
	}
}


ShaderUniformBits GetShaderUniformFromString(const char* str)
{
	ASSERT2(false, "Unrecognized value");
	return SU_NONE;
}

const char* GetShaderUniformName(ShaderUniformBits value)
{
	ASSERT2(false, "Unrecognized value");
	return "";
}


ShaderTextureBits GetShaderTextureFromString(const char* str)
{
	if(string::Compare(str, "diffuse"))
		return ST_DIFF_TEXTURE_BIT;
	else if(string::Compare(str, "normal"))
		return ST_NORM_TEXTURE_BIT;
	else
		return ST_NONE;
}

const char* GetShaderTextureName(ShaderTextureBits value)
{
	if(value == ST_DIFF_TEXTURE_BIT)
		return "diffuse";
	else if(value == ST_NORM_TEXTURE_BIT)
		return "normal";
	else
	{
		ASSERT2(false, "Unrecognized value");
		return "";
	}
}


}