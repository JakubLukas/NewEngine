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

	Quaternion(const Quaternion& other);
	Quaternion& operator=(const Quaternion& other);

	static Vector3 Multiply(const Quaternion& quat, const Vector3& vec);

	float Length() const;

	void Normalize();
	Quaternion Normalized() const;

	void Inverse();
	Quaternion Inversed() const;


	float x;
	float y;
	float z;
	float w;
};

Quaternion operator*(const Quaternion& q1, const Quaternion& q2);


}