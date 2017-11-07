#pragma once


namespace Veng
{


const float PI = 3.1415926535897932384626433832795f;


float sinf(float num);
float asinf(float num);

float cosf(float num);
float acosf(float num);

float tanf(float num);
float atanf(float num);

float pow(float num, float exp);
float sqrtf(float num);

inline float toRad(float deg)
{
	return deg * PI / 180.0f;
}

inline float toDeg(float rad)
{
	return rad * 180.0f / PI;
}


}