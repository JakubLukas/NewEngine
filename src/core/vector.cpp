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

Vector3& Vector3::operator=(const Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}



Vector4::Vector4()
	: x(0.0f)
	, y(0.0f)
	, z(0.0f)
	, w(1.0f)
{
}

Vector4::Vector4(float x, float y, float z, float w)
	: x(x)
	, y(y)
	, z(z)
	, w(w)
{
}

Vector4::Vector4(const Vector3& vec3, float w)
	: x(vec3.x)
	, y(vec3.y)
	, z(vec3.z)
	, w(w)
{
}

Vector4::Vector4(const Vector4& vec)
	: x(vec.x)
	, y(vec.y)
	, z(vec.z)
	, w(vec.w)
{

}

Vector4& Vector4::operator=(const Vector4& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	w = vec.w;
	return *this;
}


Vector3& Vector3::operator =(const Vector3& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	return *this;
}


}