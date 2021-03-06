#pragma once


namespace Veng
{


constexpr inline float MSecFromSec(float t)
{
	return t * 1000.0f;
}

constexpr inline float MSecFromMin(float t)
{
	return t * 1000.0f * 60.0f;
}

constexpr inline float MSecFromHod(float t)
{
	return t * 1000.0f * 60.0f * 60.0f;
}


constexpr inline float SecFromMSec(float t)
{
	return t * 0.001f;
}

constexpr inline float SecFromMin(float t)
{
	return t * 60.0f;
}

constexpr inline float SecFromHod(float t)
{
	return t * 60.0f * 60.0f;
}


constexpr inline float MinFromMSec(float t)
{
	return t * 0.001f * 0.0166666666666666f;
}

constexpr inline float MinFromSec(float t)
{
	return t * 0.0166666666666666f;
}

constexpr inline float MinFromHod(float t)
{
	return t * 60.0f;
}


constexpr inline float operator "" _msec(long double t)
{
	return (float)t;
}

constexpr inline float operator "" _sec(long double t)
{
	return (float)(t * 1000.0);
}

constexpr inline float operator "" _min(long double t)
{
	return (float)(t * 1000.0 * 60.0f);
}

constexpr inline float operator "" _hod(long double t)
{
	return (float)(t * 1000.0 * 60.0f * 60.0f);
}


}