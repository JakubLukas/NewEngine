#include "vector.h"


namespace Veng
{


Vector3::Vector3()
	: x(0.0f)
	, y(0.0f)
	, z(0.0f)
{
}


Vector3::Vector3(float x, float y, float z)
	: x(x)
	, y(y)
	, z(z)
{
}


Vector3::Vector3(const Vector3& vec)
	: x(vec.x)
	, y(vec.y)
	, z(vec.z)
{
}


}