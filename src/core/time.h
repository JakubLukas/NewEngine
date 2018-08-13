#pragma once


namespace Veng
{


constexpr float MSecFromSec(float t)
{
	return t * 1000.0f;
}

constexpr float MSecFromMin(float t)
{
	return t * 1000.0f * 60.0f;
}

constexpr float MSecFromHod(float t)
{
	return t * 1000.0f * 60.0f * 60.0f;
}


constexpr float SecFromMSec(float t)
{
	return t * 0.001f;
}

constexpr float SecFromMin(float t)
{
	return t * 60.0f;
}

constexpr float SecFromHod(float t)
{
	return t * 60.0f * 60.0f;
}


constexpr float MinFromMSec(float t)
{
	return t * 0.001f * 0.0166666666666666f;
}

constexpr float MinFromSec(float t)
{
	return t * 0.0166666666666666f;
}

constexpr float MinFromHod(float t)
{
	return t * 60.0f;
}


constexpr float operator "" _msec(long double t)
{
	return (float)t;
}

constexpr float operator "" _sec(long double t)
{
	return (float)(t * 1000.0);
}

constexpr float operator "" _min(long double t)
{
	return (float)(t * 1000.0 * 60.0f);
}

constexpr float operator "" _hod(long double t)
{
	return (float)(t * 1000.0 * 60.0f * 60.0f);
}


}