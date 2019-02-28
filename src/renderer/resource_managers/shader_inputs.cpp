#include "shader_inputs.h"

#include "core/asserts.h"
#include "core/string.h"


namespace Veng
{


ShaderVaryingBits GetShaderVaryingFromString(const char* str)
{
	if(string::Compare(str, "position") == 0)
		return ShaderVarying_Position;
	else if(string::Compare(str, "color0") == 0)
		return ShaderVarying_Color0;
	else if(string::Compare(str, "texcoord0") == 0)
		return ShaderVarying_Texcoords0;
	else if(string::Compare(str, "texcoord1") == 0)
		return ShaderVarying_Texcoords1;
	else if(string::Compare(str, "normal") == 0)
		return ShaderVarying_Normal;
	else if(string::Compare(str, "tangent") == 0)
		return ShaderVarying_Tangent;
	else
	{
		ASSERT2(false, "Unrecognized value");
		return ShaderVarying_None;
	}
}

const char* GetShaderVaryingName(ShaderVaryingBits value)
{
	if(value == ShaderVarying_Position)
		return "position";
	else if(value == ShaderVarying_Color0)
		return "color0";
	else if(value == ShaderVarying_Texcoords0)
		return "texcoord0";
	else if(value == ShaderVarying_Texcoords1)
		return "texcoord1";
	else if(value == ShaderVarying_Normal)
		return "normal";
	else if(value == ShaderVarying_Tangent)
		return "tangent";
	else
	{
		ASSERT2(false, "Unrecognized value");
		return "";
	}
}


ShaderUniformBits GetShaderUniformFromString(const char* str)
{
	ASSERT2(false, "Unrecognized value");
	return ShaderUniform_None;
}

const char* GetShaderUniformName(ShaderUniformBits value)
{
	ASSERT2(false, "Unrecognized value");
	return "";
}


}
