#include "math_ext.h"

#include "math.h"


namespace Veng
{


Vector3 CortesianToBarycentric(const Vector3& p, const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	//TODO assert, points must be on one plane

	float detTInv = 1.0f / ((v1.x - v3.x)*(v2.y - v3.y)) - ((v2.x - v3.x)*(v1.y - v3.y));
	float a1 = ((v2.y - v3.y)*(p.x - v3.x) + (v3.x - v2.x)*(p.y - v3.y)) * detTInv;
	float a2 = ((v3.y - v1.y)*(p.x - v3.x) + (v1.x - v3.x)*(p.y - v3.y)) * detTInv;
	float a3 = 1.0f - a1 - a2;

	return {a1, a2, a3};

}


bool IsPointInsideTriangle(const Vector3& point, const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	//TODO we don't need full barycentric calculation, if one weight is lower than 0, it's outside

	Vector3 barycentric = CortesianToBarycentric(point, v1, v2, v3);

	return (barycentric.x >= 0.0f && barycentric.y >= 0.0f && barycentric.z >= 0.0f);
}


void ToRad(Vector3& angles)
{
	angles.x = toRad(angles.x);
	angles.y = toRad(angles.y);
	angles.z = toRad(angles.z);
}

void ToDeg(Vector3& angles)
{
	angles.x = toDeg(angles.x);
	angles.y = toDeg(angles.y);
	angles.z = toDeg(angles.z);
}


void QuatToEuler(const Quaternion& quat, Vector3& euler)
{
	//yaw
	float sinp = 2.0f * (quat.w * quat.y - quat.z * quat.x);
	if(absf(sinp) >= 1.0f)
		euler.x = copysignf(PI_HALF, sinp); // use 90 degrees if out of range
	else
		euler.x = asinf(sinp);

	//pitch
	float sinr_cosp = 2.0f * (quat.w * quat.x + quat.y * quat.z);
	float cosr_cosp = 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y);
	euler.y = atan2f(sinr_cosp, cosr_cosp);

	//roll
	float siny_cosp = 2.0f * (quat.w * quat.z + quat.x * quat.y);
	float cosy_cosp = 1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z);
	euler.z = atan2f(siny_cosp, cosy_cosp);
}

void EulerToQuat(const Vector3& euler, Quaternion& quat)
{
	float cR = cosf(euler.z * 0.5f);
	float sR = sinf(euler.z * 0.5f);
	float cP = cosf(euler.x * 0.5f);
	float sP = sinf(euler.x * 0.5f);
	float cY = cosf(euler.y * 0.5f);
	float sY = sinf(euler.y * 0.5f);

	quat.w = cR * cP * cY + sR * sP * sY;
	quat.x = cR * cP * sY - sR * sP * cY;
	quat.y = sR * cP * sY + cR * sP * cY;
	quat.z = sR * cP * cY - cR * sP * sY;
}


}