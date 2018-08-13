#pragma once


namespace Veng
{


constexpr float PI = 3.1415926535897932384626433832795f;


float sinf(float num);
float asinf(float num);

float cosf(float num);
float acosf(float num);

float tanf(float num);
float atanf(float num);

float pow(float num, float exp);
float sqrtf(float num);


float min(float v1, float v2);
float max(float v1, float v2);

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