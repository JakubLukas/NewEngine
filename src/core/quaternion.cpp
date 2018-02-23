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