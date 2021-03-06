#pragma once


namespace Veng
{


template<class Type>
Type Min(Type v1, Type v2)
{
	return (v1 < v2) ? v1 : v2;
}

template<class Type>
Type Max(Type v1, Type v2)
{
	return (v1 > v2) ? v1 : v2;
}


float copysignf(float num, float sign);


constexpr float PI = 3.1415926535897932384626433832795f;
constexpr float PI_HALF = PI / 2.0f;

float absf(float num);

float sinf(float num);
float asinf(float num);

float cosf(float num);
float acosf(float num);

float tanf(float num);
float atanf(float num);
float atan2f(float y, float x);

float powf(float num, float exp);
float sqrtf(float num);


constexpr float toRad(float deg)
{
	return deg * PI / 180.0f;
}

constexpr float toDeg(float rad)
{
	return rad * 180.0f / PI;
}

constexpr float operator "" _rad(long double rad)
{
	return (float)rad;
}

constexpr float operator "" _deg(long double deg)
{
	return (float)(deg * PI / 180.0);
}


}