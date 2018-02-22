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


Matrix44 Quaternion::ToMatrix44() const
{
	float fx = x + x;
	float fy = y + y;
	float fz = z + z;
	float fwx = fx * w;
	float fwy = fy * w;
	float fwz = fz * w;
	float fxx = fx * x;
	float fxy = fy * x;
	float fxz = fz * x;
	float fyy = fy * y;
	float fyz = fz * y;
	float fzz = fz * z;

	Matrix44 mtx;
	mtx.m11 = 1.0f - (fyy + fzz);
	mtx.m21 = fxy - fwz;
	mtx.m31 = fxz + fwy;
	mtx.m12 = fxy + fwz;
	mtx.m22 = 1.0f - (fxx + fzz);
	mtx.m32 = fyz - fwx;
	mtx.m13 = fxz - fwy;
	mtx.m23 = fyz + fwx;
	mtx.m33 = 1.0f - (fxx + fyy);

	mtx.m41 = mtx.m42 = mtx.m43 = mtx.m14 = mtx.m24 = mtx.m34 = 0.0f;
	mtx.m44 = 1.0f;
	return mtx;
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