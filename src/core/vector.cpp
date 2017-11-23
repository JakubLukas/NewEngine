#include "vector.h"

#include "math.h"


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


Vector3 Vector3::operator+(const Vector3& other) const
{
	return Vector3(
		x + other.x,
		y + other.y,
		z + other.z
	);
}

Vector3 Vector3::operator-(const Vector3& other) const
{
	return Vector3(
		x - other.x,
		y - other.y,
		z - other.z
	);
}


float Vector3::Length() const
{
	return sqrtf(Dot(*this, *this));
}


void Vector3::Normalize()
{
	float invLength = 1.0f / Length();
	x *= invLength;
	y *= invLength;
	z *= invLength;
}


float Vector3::Dot(const Vector3& vec1, const Vector3& vec2)
{
	return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}

Vector3 Vector3::Cross(const Vector3& vec1, const Vector3& vec2)
{
	return Vector3(
		vec1.y * vec2.z - vec1.z * vec2.y,
		vec1.z * vec2.x - vec1.x * vec2.z,
		vec1.x * vec2.y - vec1.y * vec2.x
	);
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


}