#pragma once

#include "math/vector.h"
#include "entity.h"


namespace Veng
{


struct Ray
{
	Vector3 origin;
	Vector3 direction;
};


struct RayHit
{
	Entity entity;
	Vector3 intersection;
	Vector3 normal;
};


}