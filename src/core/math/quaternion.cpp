#include "quaternion.h"

#include "math.h"


namespace Veng
{


const Quaternion Quaternion::IDENTITY(0.0f, 0.0f, 0.0f, 1.0f);


Quaternion::Quaternion()
{
}

Quaternion::Quaternion(const Vector3& axis, float angle)
{
	float half_angle = angle * 0.5f;
	float s = sinf(half_angle);
	w = cosf(half_angle);
	x = axis.x * s;
	y = axis.y * s;
	z = axis.z * s;
}

Quaternion::Quaternion(float x, float y, float z, float w)
	: x(x), y(y), z(z), w(w)
{
}

Quaternion::Quaternion(const Quaternion& other)
	: w(other.w)
	, x(other.x)
	, y(other.y)
	, z(other.z)
{}

Quaternion& Quaternion::operator=(const Quaternion& other)
{
	w = other.w;
	x = other.x;
	y = other.y;
	z = other.z;
	return *this;
}


Vector3 Quaternion::Multiply(const Quaternion& quat, const Vector3& vec)
{
	Quaternion result = quat * Quaternion(vec.x, vec.y, vec.z, 0.0f) * quat.Inversed();
	return Vector3(result.x, result.y, result.z);
}


float Quaternion::Length() const
{
	return sqrtf(w*w + x*x + y*y + z*z);
}

void Quaternion::Normalize()
{
	float lenInv = 1.0f / Length();
	w *= lenInv;
	x *= lenInv;
	y *= lenInv;
	z *= lenInv;
}

Quaternion Quaternion::Normalized() const
{
	float lenInv = 1.0f / Length();
	return Quaternion(
		x * lenInv,
		y * lenInv,
		z * lenInv,
		w * lenInv
	);
}


void Quaternion::Inverse()
{
	float lenInv = 1.0f / Length();
	w *= lenInv;
	x *= -lenInv;
	y *= -lenInv;
	z *= -lenInv;
}

Quaternion Quaternion::Inversed() const
{
	float lenInv = 1.0f / Length();
	return Quaternion(
		-x * lenInv,
		-y * lenInv,
		-z * lenInv,
		w * lenInv
	);
}


Quaternion operator*(const Quaternion& q1, const Quaternion& q2)
{
	return Quaternion(
		q1.w * q2.x + q2.w * q1.x + q1.y * q2.z - q2.y * q1.z,
		q1.w * q2.y + q2.w * q1.y + q1.z * q2.x - q2.z * q1.x,
		q1.w * q2.z + q2.w * q1.z + q1.x * q2.y - q2.x * q1.y,
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z

	);
}


}
