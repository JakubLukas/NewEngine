#pragma once

#include "core/int.h"
#include "core/math/vector.h"
#include "core/color.h"


namespace Veng
{


enum class LightType : u8
{
	Ambient,
	Directional,
	Point,
	Spot,

	Count
};


struct AmbientLight
{
	Color color;
};


struct DirectionalLight
{
	Color diffuseColor;
	Color specularColor;
	u32 unused1;
	u32 unused2;
};


struct PointLight
{
	Color color;
	float attenuation;
};


struct SpotLight
{
	Color color;
	float cosineCutoff;
	float exponent;
	float attenuation;
};


}