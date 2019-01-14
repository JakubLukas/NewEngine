#pragma once

#include "core/int.h"
#include "core/math/vector.h"
#include "renderer/color.h"


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
	Color color;
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