#include "vector.h"

#include "math.h"
#include "core/asserts.h"


namespace Veng
{


const Vector2 Vector2::AXIS_X(1.0f, 0.0f);
const Vector2 Vector2::AXIS_Y(0.0f, 1.0f);

Vector2::Vector2()
	: x(0.0f)
	, y(0.0f)
{}

Vector2::Vector2(float x, float y)
	: x(x)
	, y(y)
{}

Vector2::Vector2(const Vector2& vec)
	: x(vec.x)
	, y(vec.y)
{}

Vector2& Vector2::operator=(const Vector2& other)
{
	x = other.x;
	y = other.y;
	return *this;
}


Vector2 Vector2::operator+() const
{
	return Vector2(*this);
}

Vector2 Vector2::operator-() const
{
	return Vector2(-x, -y);
}


Vector2 Vector2::operator+(const Vector2& other) const
{
	return Vector2(
		x + other.x,
		y + other.y
	);
}
Vector2 Vector2::operator-(const Vector2& other) const
{
	return Vector2(
		x - other.x,
		y - other.y
	);
}



const Vector3 Vector3::AXIS_X(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::AXIS_Y(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::AXIS_Z(0.0f, 0.0f, 1.0f);


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

Vector3::Vector3(const Vector2& vec2, float z)
	: x(vec2.x)
	, y(vec2.y)
	, z(z)
{}

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


Vector3 Vector3::operator+() const
{
	return Vector3(*this);
}

Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
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


Vector3 operator*(float value, const Vector3& vec)
{
	return Vector3(
		value * vec.x,
		value * vec.y,
		value * vec.z
	);
}

Vector3 operator*(const Vector3& vec, float value)
{
	return Vector3(
		value * vec.x,
		value * vec.y,
		value * vec.z
	);
}

Vector3 operator/(float value, const Vector3& vec)
{
	float valueInv = 1.0f / value;
	return Vector3(
		valueInv * vec.x,
		valueInv * vec.y,
		valueInv * vec.z
	);
}
Vector3 operator/(const Vector3& vec, float value)
{
	float valueInv = 1.0f / value;
	return Vector3(
		valueInv * vec.x,
		valueInv * vec.y,
		valueInv * vec.z
	);
}



const Vector4 Vector4::AXIS_X(1.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::AXIS_Y(0.0f, 1.0f, 0.0f, 0.0f);
const Vector4 Vector4::AXIS_Z(0.0f, 0.0f, 1.0f, 0.0f);


Vector4::Vector4()
	: x(0.0f)
	, y(0.0f)
	, z(0.0f)
	, w(0.0f)
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


Vector4 Vector4::operator+(const Vector4& other) const
{
	return Vector4(
		x + other.x,
		y + other.y,
		z + other.z,
		w + other.w
	);
}

Vector4 Vector4::operator-(const Vector4& other) const
{
	return Vector4(
		x - other.x,
		y - other.y,
		z - other.z,
		w - other.w
	);
}


float Vector4::Length() const
{
	return sqrtf(Dot(*this, *this));
}


void Vector4::Normalize()
{
	float invLength = 1.0f / Length();
	x *= invLength;
	y *= invLength;
	z *= invLength;
}


Vector3 Vector4::GetXYZ() const
{
	return Vector3(x, y, z);
}

Vector3 Vector4::GetVector3() const
{
	ASSERT(-0.0001f < w && w < 0.0001f);
	float wInv = 1.0f / w;
	return Vector3(x * wInv, y * wInv, z * wInv);
}


float Vector4::Dot(const Vector4& vec1, const Vector4& vec2)
{
	return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z + vec1.w * vec2.w;
}


Vector4 operator*(float value, const Vector4& vec)
{
	return Vector4(
		value * vec.x,
		value * vec.y,
		value * vec.z,
		value * vec.w
	);
}

Vector4 operator*(const Vector4& vec, float value)
{
	return Vector4(
		value * vec.x,
		value * vec.y,
		value * vec.z,
		value * vec.w
	);
}

Vector4 operator/(float value, const Vector4& vec)
{
	float valueInv = 1.0f / value;
	return Vector4(
		valueInv * vec.x,
		valueInv * vec.y,
		valueInv * vec.z,
		valueInv * vec.w
	);
}

Vector4 operator/(const Vector4& vec, float value)
{
	float valueInv = 1.0f / value;
	return Vector4(
		valueInv * vec.x,
		valueInv * vec.y,
		valueInv * vec.z,
		valueInv * vec.w
	);
}


}