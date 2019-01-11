#pragma once

#include "core/int.h"
#include "core/math/vector.h"
#include "renderer/color.h"


namespace Veng
{


enum class LightType : u8
{
	Directional,
	Point,
	Spot,
};


struct DirectionalLight
{
	Color color;
	Vector3 direction;
};


struct PointLight
{
	Color color;
	float attenuation;
};


struct SpotLight
{
	Color color;
	Vector3 direction;
	float cosineCutoff;
	float exponent;
	float attenuation;
};


}