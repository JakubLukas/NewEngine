#include "math_ext.h"


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


}