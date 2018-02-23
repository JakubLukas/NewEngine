#pragma once

#include "vector.h"


namespace Veng
{


struct Quaternion
{
	static const Quaternion IDENTITY;

	Quaternion();
	Quaternion(const Vector3& axis, float angle);
	Quaternion(float x, float y, float z, float w);


	float x;
	float y;
	float z;
	float w;
};

Quaternion operator*(const Quaternion& q1, const Quaternion& q2);


}